// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "MurphysLaw.h"
#include "MurphysLawCharacter.h"
#include "../Network/MurphysLawPlayerState.h"
#include "../Weapon/MurphysLawBaseWeapon.h"
#include "../HUD/MurphysLawHUDWidget.h"
#include "../Components/MurphysLawInventoryComponent.h"
#include "../Menu/MurphysLawInGameMenu.h"
#include "../Network/MurphysLawPlayerController.h"
#include "../AI/MurphysLawAIController.h"
#include "WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Animation/AnimInstance.h"
#include "UnrealNetwork.h"
#include "../DamageZone/MurphysLawDamageZone.h"
#include "../Environnement/ExplosiveBarrel/MurphysLawExplosiveBarrel.h"
#include "MurphysLawNameplateWidget.h"
#include "Kismet/KismetMathLibrary.h"
#include "../Network/MurphysLawGameMode.h"
#include "../Network/MurphysLawGameState.h"

#include <MurphysLaw/Interface/MurphysLawIController.h>
#include <MurphysLaw/Utils/MurphysLawUtils.h>

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AMurphysLawCharacter

const float AMurphysLawCharacter::DefaultAimFactor = 90.0f;
const FName AMurphysLawCharacter::MATERIAL_PARAM_TEAM_COLOR_CLOTHES("TeamClothesColor");
const FName AMurphysLawCharacter::MATERIAL_PARAM_TEAM_COLOR_MASK("TeamMaskColor");
const float AMurphysLawCharacter::ROTATION_RATE_HUMAN(360.f);
const float AMurphysLawCharacter::ROTATION_RATE_BOT(160.f);
const float AMurphysLawCharacter::FACTOR_HEADSHOT(3.f);
const float AMurphysLawCharacter::FACTOR_CHESTSHOT(1.5f);
const FString AMurphysLawCharacter::SOCKET_HEAD = "Head";
const FString AMurphysLawCharacter::SOCKET_SPINE = "Spine1";

AMurphysLawCharacter::AMurphysLawCharacter()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->AttachParent = GetCapsuleComponent();
	FirstPersonCameraComponent->RelativeLocation = FVector(0, 0, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a SceneComponent
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SceneComponent->AttachParent = GetCapsuleComponent();
	SceneComponent->RelativeLocation = FVector(-20, 0, 120);

	// Create a CharacterNameplate	
	CharacterNameplate = CreateDefaultSubobject<UWidgetComponent>(TEXT("CharacterNameplate"));
	CharacterNameplate->AttachParent = SceneComponent;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->AttachParent = FirstPersonCameraComponent;
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;

	GetMesh()->SetCollisionObjectType(ECC_PhysicsBody);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	GetMesh()->SetCollisionResponseToAllChannels(ECR_Block);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P are set in the
	// derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	// Set base health level for the character
	MaxHealth = 100.f;
	Dead = false;
	IsRunning = false;

	// Set the Index so the character starts with no weapon
	CurrentWeaponIndex = NO_WEAPON_VALUE;

	// Creates the inventory component to store the weapons of the character
	Inventory = CreateDefaultSubobject<UMurphysLawInventoryComponent>(TEXT("InventoryComponent"));
	checkf(Inventory != nullptr, TEXT("Inventory has not been initialized correctly"));

	// Movement related
	IsCrouched = false;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	ConfigureMovement(false); // Configure as human to see human related values in editor

	// AI and possessing settings from APawm
	AIControllerClass = nullptr; // Should be a subclass of AMurphysLawAIController
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AutoPossessPlayer = EAutoReceiveInput::Disabled;

	// Spawning settings
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Try to load the sound of switching weapon
	static ConstructorHelpers::FObjectFinder<USoundBase> SwitchWeaponSound(TEXT("SoundWave'/Game/MurphysLawRessources/Weapons/S_Switch_Weapon.S_Switch_Weapon'"));
	SwitchingWeaponSound = nullptr;
	if (SwitchWeaponSound.Succeeded())
	{
		SwitchingWeaponSound = SwitchWeaponSound.Object;
	}

	// Sets up the stamina info
	MaxStamina = 100.f;
	RunningStaminaDecayRate = 0.4f;
	JumpStaminaDecayAmount = 7.5f;
	StaminaRegenerationRate = 0.07f;

	// Team color flags
	ValidTeamBodyMeshColor = false;
	ValidTeamMaskMeshColor = false;
}

// Indicates to the server what properties of the object to replicate on the clients
void AMurphysLawCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMurphysLawCharacter, CurrentWeaponIndex);
	DOREPLIFETIME(AMurphysLawCharacter, CurrentHealth);
	DOREPLIFETIME(AMurphysLawCharacter, Dead);
	DOREPLIFETIME(AMurphysLawCharacter, TeamBodyMeshColor);
	DOREPLIFETIME(AMurphysLawCharacter, TeamMaskMeshColor);
}

void AMurphysLawCharacter::BeginPlay()
{
	Super::BeginPlay();

	// AI settings ressources
	if (AIControllerClass == nullptr) ShowWarning("No AIController class assigned to MurphysLawCharacter");

	if (SwitchingWeaponSound == nullptr) ShowWarning("MurphysLawCharacter - Unable to load the SwitchingWeaponSound");

	// If the InventoryComponent's BeginPlay has not been called yet, we call it
	if (!Inventory->HasBegunPlay())
	{
		Inventory->BeginPlay();
	}

	EquipFirstWeapon();

	// Set the current health of the character here in case the value has been overriden in a subclass
	CurrentHealth = MaxHealth;

	auto NameplateWidget = Cast<UMurphysLawNameplateWidget>(CharacterNameplate->GetUserWidgetObject());
	if (NameplateWidget)
	{
		NameplateWidget->SetCharacter(this);
	}

	// Sets the current stamina level to the maximum
	CurrentStamina = MaxStamina;
}

// Called when game ends
void AMurphysLawCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Clear the inventory
	if (Inventory->HasBegunPlay()) Inventory->EndPlay(EndPlayReason);
}

// Called when the character is possessed by a new controller
void AMurphysLawCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (NewController != nullptr)
	{
		// Update settings of movement component to allow "natural" movement for bots
		const bool ConfigureAsBot = NewController->IsA<AAIController>();
		ConfigureMovement(ConfigureAsBot);
	}
}

/** Configure character movement with human or bot specific settings */
void AMurphysLawCharacter::ConfigureMovement(const bool ConfigureForBot)
{
	if (ConfigureForBot)
	{	// Bot movement configuration

		// Allow looking in different direction than movement
		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;

		// Change left/right rotation speed
		GetCharacterMovement()->RotationRate.Yaw = ROTATION_RATE_BOT;
	}
	else
	{	// Human movement configuration

		// Allow looking in different direction than movement
		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;

		// Change left/right rotation speed
		GetCharacterMovement()->RotationRate.Yaw = ROTATION_RATE_HUMAN;
	}
}

/** Defines a world-space point where an ai should look
	Since the gun is located in the center of the body, it is a good focal point */
AActor* AMurphysLawCharacter::GetFocalPoint() const { return GetEquippedWeapon(); }

void AMurphysLawCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (FirstPersonCameraComponent != nullptr)
	{
		// Gets the camera angle for the mini-map (to rotate it as the player rotates the player)
		Bearing = FirstPersonCameraComponent->GetComponentRotation().Yaw;
	}

	// Calculate the current regeneration rate according on character's movement
	const bool IsMoving = GetVelocity().Size() != 0.f;
	const float CurrentRegenerationRate = IsMoving ? StaminaRegenerationRate : StaminaRegenerationRate * 2;

	// Raise the level of stamina overtime
	UpdateStaminaLevel(CurrentRegenerationRate * DeltaSeconds * GetMaxStaminaLevel());

	// If the character is not moving, we deactivate his running so he doesn't lose stamina
	if (!IsMoving)
	{
		SetIsRunning(false);
	}

	// But lower the stamina if the player is running
	if (IsRunning)
	{
		UpdateStaminaLevel(-RunningStaminaDecayRate * DeltaSeconds * GetMaxStaminaLevel());

		// If the character is out of breath, it stops running
		if (GetCurrentStaminaLevel() <= 0.f)
		{
			SetIsRunning(false);
		}
	}

	if (this->GetMovementComponent()->IsFalling())
	{
		SetIsInAir(true);
		HighestZ = FMath::Max(HighestZ, GetActorLocation().Z);
	}
	else
	{
		float DeltaZ = (HighestZ - GetActorLocation().Z) / 10.f;
		//On ne veut pas de dégât pour les petits sauts...
		//Pour calculer le dégât fait en sautant de la tour
		if (DeltaZ > 100)
		{
			FHitResult HitResult;
			float Damage = DeltaZ * 0.35f;
			FVector HurtDirection = GetActorLocation();
			FPointDamageEvent CollisionDamageEvent(Damage, HitResult, HurtDirection, UDamageType::StaticClass());

			if (Role == ROLE_Authority)
			{
				TakeDamage(Damage, CollisionDamageEvent, GetController(), this);
			}
		}

		//Pour calculer le dégât fait en sautant du balcon
		else if (DeltaZ > 30)
		{
			FHitResult HitResult;
			float Damage = DeltaZ * 0.5f;
			FVector HurtDirection = GetActorLocation();
			FPointDamageEvent CollisionDamageEvent(Damage, HitResult, HurtDirection, UDamageType::StaticClass());

			if (Role == ROLE_Authority)
			{
				TakeDamage(Damage, CollisionDamageEvent, GetController(), this);
			}
		}

		HighestZ = 0;
		SetIsInAir(false);
	}
	/** [PS] DO NOT REMOVE - Trying to make it work */

	//ACharacter* myCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	//if (myCharacter) //Maybe check character
	//{
	//	FVector charLocation = myCharacter->GetActorLocation();
	//	FVector sceneLocation = SceneComponent->GetComponentLocation();
	//
	//	FRotator PlayerRot = UKismetMathLibrary::FindLookAtRotation(sceneLocation, charLocation);
	//	float X, Y, Z;

	//	UKismetMathLibrary::BreakRotator(PlayerRot, X, Y, Z);
	//	FRotator Result = UKismetMathLibrary::MakeRotator(0.f, 0.f, Z);
	//	//UKismetMathLibrary::BreakRotIntoAxes(PlayerRot, X, Y, Z);
	//	
	//	SceneComponent->SetWorldLocationAndRotation(sceneLocation, Result);
	//	/*SceneComponent->SetWorldRotation(FRotationMatrix::MakeFromXY(X, Y).ToQuat());*/
	//}
}

//////////////////////////////////////////////////////////////////////////
// Input

bool AMurphysLawCharacter::IsInAir() const
{
	return InAir;
}

void AMurphysLawCharacter::SetIsInAir(bool isInAir)
{
	InAir = isInAir;
}

void AMurphysLawCharacter::Aim()
{
	if (HasWeaponEquipped())
	{
		FirstPersonCameraComponent->FieldOfView = GetEquippedWeapon()->AimFactor;
		IsCharacterAiming = true;
	}
}

void AMurphysLawCharacter::StopAiming()
{
	if (HasWeaponEquipped())
	{
		FirstPersonCameraComponent->FieldOfView = DefaultAimFactor;
		IsCharacterAiming = false;
	}
}

void AMurphysLawCharacter::MoveForward(float Value)
{
	if (Value != 0.0f && CanPlayerMove())
	{
		// If the character is going backward, he can't be running
		if (Value < 0.f)
		{
			SetIsRunning(false);
		}

		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AMurphysLawCharacter::MoveRight(float Value)
{
	if (Value != 0.0f && CanPlayerMove())
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AMurphysLawCharacter::Run()
{
	SetIsRunning(true);
}

void AMurphysLawCharacter::StopRunning()
{
	SetIsRunning(false);
}
///////////////////////////////////////////////////////////////////////////

#pragma region Health functions

// Reports current health level of the character
float AMurphysLawCharacter::GetCurrentHealthLevel() const { return CurrentHealth; }

// Reports maximum health level of the character
float AMurphysLawCharacter::GetMaxHealthLevel() const { return MaxHealth; }

// Increase the health of the character
void AMurphysLawCharacter::ReceiveHealAmount(const float HealingAmount)
{
	checkf(HealingAmount >= 0, TEXT("HealingAmount needs to be higher than zero"));
	CurrentHealth = FMath::Min(CurrentHealth + HealingAmount, MaxHealth);
}

#pragma endregion

// Refills ammos for the current equipped weapon
void AMurphysLawCharacter::ReceiveAmmo(const int32 NumberOfAmmo)
{
	GetEquippedWeapon()->AddAmmoInInventory(NumberOfAmmo);

	// If the equipped weapon is empty when picking up ammos, it auto-reloads
	if (ShouldReload())
	{
		Reload();
	}
}

// Refills ammos for the collected weapon or collects it if character didn't have it yet
void AMurphysLawCharacter::CollectWeapon(class AMurphysLawBaseWeapon* Weapon)
{
	Inventory->CollectWeapon(Weapon);

	// If the equipped weapon is empty when picking up the weapon and it's the same, it auto-reloads
	if (HasWeaponEquipped()
		&& GetEquippedWeapon()->IsOfSameType(Weapon)
		&& ShouldReload())
	{
		Reload();
	}
}

// Makes the character die
void AMurphysLawCharacter::Die()
{
	Dead = true;

	IMurphysLawIController* PlayerController = Cast<IMurphysLawIController>(Controller);
	if (PlayerController)
	{
		PlayerController->OnKilled(TimeToRespawn);
	}

	// Disable collisions for the actor as he's dead
	SetActorEnableCollision(false);

	// Hide the current weapon of the player before destroying it
	if (HasWeaponEquipped())
	{
		GetEquippedWeapon()->SetActorHiddenInGame(true);
		Inventory->GetFullMeshWeapon(CurrentWeaponIndex)->SetActorHiddenInGame(true);
	}
}

bool AMurphysLawCharacter::CanPlayerMove()
{
	bool CanMove = true;

	AMurphysLawBaseWeapon* CurrentWeapon = GetEquippedWeapon();
	if (CurrentWeapon)
		CanMove = !(IsCrouched && CurrentWeapon->IsReloading);

	return CanMove;
}

// Makes the character live again
void AMurphysLawCharacter::Relive()
{
	Dead = false;
	CurrentHealth = MaxHealth;
	Inventory->Reinitialize();
	EquipFirstWeapon();

	// Enable collisions for the actor as he's alive
	SetActorEnableCollision(true);
}

// Executed when the Dead variable is replicated
void AMurphysLawCharacter::OnRep_Dead()
{
	// When the character is revived by the server, we reinitialize his inventory
	if (!Dead)
	{
		Inventory->Reinitialize();
		EquipFirstWeapon();

		// Show HUD
		auto PlayerController = Cast<AMurphysLawPlayerController>(GetController());
		if(PlayerController != nullptr)
			PlayerController->ChangeHUDVisibility(ESlateVisibility::Visible);
	}
}

void AMurphysLawCharacter::EquipFirstWeapon()
{
	CurrentWeaponIndex = NO_WEAPON_VALUE;
	// Determines the current weapon of the character
	for (int32 i = 0; i < Inventory->NumberOfWeaponInInventory && CurrentWeaponIndex == NO_WEAPON_VALUE; ++i)
	{
		AMurphysLawBaseWeapon* Weapon = Inventory->GetWeapon(i);
		if (Weapon != nullptr)
		{
			// The first available weapon in the inventory become the current weapon
			CurrentWeaponIndex = i;
			Weapon->SetActorHiddenInGame(false);
			Inventory->GetFullMeshWeapon(i)->SetActorHiddenInGame(false);
		}
	}
}

// Reports if the character is dead 
bool AMurphysLawCharacter::IsDead() const
{
	return Dead;
}

#pragma region Shooting and bullet collision

void AMurphysLawCharacter::Fire()
{
	// check if we have a weapon equipped
	if (HasWeaponEquipped())
	{
		// if the weapon has been able to fire
		if (GetEquippedWeapon()->Fire(this))
		{
			// Stop the character from running
			SetIsRunning(false);

			// try and play a firing animation if specified
			if (FireAnimation != nullptr)
			{
				// Get the animation object for the arms mesh
				UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
				if (AnimInstance != nullptr)
				{
					AnimInstance->Montage_Play(FireAnimation, 1.f);
				}
			}

			// check for bullet collisions
			ComputeBulletCollisions();
		}

		// We reload the weapon if it is empty and we have bullets left in our inventory
		if (ShouldReload())
		{
			Reload();
		}
	}
}

// Check for bullet collisions
void AMurphysLawCharacter::ComputeBulletCollisions()
{
	// Only look for pawns
	FCollisionObjectQueryParams CollisionObjectQueryParams;
	CollisionObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_PhysicsBody);
	CollisionObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Destructible);
	CollisionObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);

	// Remove self from query potential results since we are the first to collide with the ray
	FCollisionQueryParams RayQueryParams;
	RayQueryParams.AddIgnoredActor(this);

	// Ray starting coordinates
	const FVector CollisionRayStart = GetFirstPersonCameraComponent()->GetComponentLocation();
	const FVector CollisionRayInitialDirection = GetFirstPersonCameraComponent()->GetComponentTransform().GetRotation().GetAxisX();

	// Group damage of touched objects together
	const float MaxFragmentDeviationRadian = FMath::DegreesToRadians(GetEquippedWeapon()->GetMaxFragmentDeviationAngle(IsCharacterAiming));

	bool AtLeastOneHit = false;

	// Trace lines to detect pawn
	for (int32 i = 0; i < GetEquippedWeapon()->GetNumberOfEmittedFragments(); ++i)
	{
		// Ray ending coordinates
		const FVector CollisionRayAngledDirection = FMath::VRandCone(CollisionRayInitialDirection, MaxFragmentDeviationRadian);
		const FVector CollisionRayEnd = CollisionRayStart + (CollisionRayAngledDirection * GetEquippedWeapon()->GetMaxTravelDistanceOfBullet());

		FHitResult CollisionResult;
		bool HasHit = GetWorld()->LineTraceSingleByObjectType(CollisionResult, CollisionRayStart, CollisionRayEnd, CollisionObjectQueryParams, RayQueryParams);

		if (HasHit)
		{
			// Simple damage amount considering the distance to the target depending on the bone hit
			float DeliveredDamage = GetDeliveredDamage(CollisionResult);

			FPointDamageEvent CollisionDamageEvent(DeliveredDamage, CollisionResult, CollisionRayAngledDirection, UDamageType::StaticClass());

			// If the actor we hit is a hittable actor and an enemy, we have at least one hit so we'll show the Hit Marker
			if (IsHittableActor(CollisionResult.GetActor()) && !MurphysLawUtils::IsInSameTeam(CollisionResult.GetActor(), this))
			{
				AtLeastOneHit = true;
			}

			if (Role == ROLE_Authority)
			{
				CollisionResult.GetActor()->TakeDamage(DeliveredDamage, CollisionDamageEvent, GetController(), this);
			}
			else
			{
				Server_TransferDamage(CollisionResult.GetActor(), DeliveredDamage, CollisionDamageEvent, GetController(), this);
			}
		}
	}

	// If there was at least one hit, we show the HitMarker
	if (AtLeastOneHit)
	{
		auto MyController = Cast<AMurphysLawPlayerController>(GetController());
		if (MyController != nullptr && MyController->GetHUDInstance() != nullptr)
		{
			MyController->GetHUDInstance()->ShowHitMarker();
		}
	}
}

float AMurphysLawCharacter::GetDeliveredDamage(const FHitResult& CollisionResult) const
{
	float DeliveredDamage = GetEquippedWeapon()->ComputeCollisionDamage(CollisionResult.Distance);
	FString BoneName = CollisionResult.BoneName.ToString();

	if (BoneName == SOCKET_HEAD)
		DeliveredDamage *= FACTOR_HEADSHOT;
	else if (BoneName == SOCKET_SPINE)
		DeliveredDamage *= FACTOR_CHESTSHOT;

	return DeliveredDamage;
}

bool AMurphysLawCharacter::Server_TransferDamage_Validate(class AActor * DamagedActor, const float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, class AActor * DamageCauser) { return true; }
void AMurphysLawCharacter::Server_TransferDamage_Implementation(class AActor * DamagedActor, const float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, class AActor * DamageCauser)
{
	DamagedActor->TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

float AMurphysLawCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser)
{
	float ActualDamage = 0.f;

	if (CurrentHealth > 0.f)
	{
		if (Role == ROLE_Authority)
		{
			ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

			// If the player actually took damage
			if (ActualDamage > 0.f)
			{
				// If the character has a HUD, we show the damages on it
				auto MyController = Cast<AMurphysLawPlayerController>(GetController());
				if (MyController != nullptr)
				{
					// We start by getting best info on the hit
					FVector ImpulseDirection;
					FHitResult Hit;
					DamageEvent.GetBestHitInfo(this, DamageCauser, Hit, ImpulseDirection);

					// We calculate the vector from the character to the damage causer
					FVector2D HitVector = FVector2D(FRotationMatrix(GetControlRotation()).InverseTransformVector(-ImpulseDirection));
					HitVector.Normalize();

					// We compute the vector representing the ForwardVector
					FVector2D StraightVector = FVector2D(1.f, 0.f);
					StraightVector.Normalize();

					// Finally, we calculate the angle where the hit came from
					float Angle = UKismetMathLibrary::DegAcos(FVector2D::DotProduct(StraightVector, HitVector));

					// The angle ranges from -180.f to 180.f
					Angle = HitVector.Y < 0.f ? -Angle : Angle;

					// Dispatch to the controller
					MyController->ShowDamage(Angle);
				}
			}
		}
		else
		{
			// Let the server do it
			Server_TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
		}
	}

	return ActualDamage;
}


void AMurphysLawCharacter::OnReceiveAnyDamage(float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	// If it was friendly fire, we do not damage our teammate, except if it is from an explosive
	if (IsFriendlyFire(InstigatedBy) && !DamageCausedByExplosive(DamageCauser))
	{
		return;
	}

	CurrentHealth = FMath::Max(CurrentHealth - Damage, 0.f);

	if (CurrentHealth <= 0.f)
	{
		UpdateStatsOnKill(InstigatedBy, DamageCauser);
		Die();
	}
}

bool AMurphysLawCharacter::Server_TakeDamage_Validate(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser) { return true; }
void AMurphysLawCharacter::Server_TakeDamage_Implementation(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser)
{
	TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

// Update the statistics of players involved in the death
void AMurphysLawCharacter::UpdateStatsOnKill(AController* InstigatedBy, AActor* DamageCauser)
{
	AMurphysLawGameState* GameState = GetWorld()->GetGameState<AMurphysLawGameState>();
	FString DeathMessage = "";
	// Try to cast the DamageCauser to DamageZone to see if the player committed suicide
	auto DamageZone = Cast<AMurphysLawDamageZone>(DamageCauser);

	// Does the player committed suicide?
	if (InstigatedBy == GetController() || DamageCausedByDamageZone(DamageCauser))
	{
		InstigatedBy = GetController();
		DeathMessage = FString::Printf(TEXT("%s committed suicide."), *GetHumanReadableName());
		if (GetPlayerState())
			GetPlayerState()->IncrementNbDeaths();
		if (GameState)
			GameState->PlayerCommitedSuicide(GetPlayerState()->GetTeam() == AMurphysLawGameMode::TEAM_A);
	}
	else if (IsFriendlyFire(InstigatedBy) && DamageCausedByExplosive(DamageCauser))
	{
		// Or was he killed by a teammate because of explosion?
		DeathMessage = FString::Printf(TEXT("%s was killed by a teammate."), *GetHumanReadableName());
		if (GetPlayerState())
			GetPlayerState()->IncrementNbDeaths();

		if (GameState)
			GameState->PlayerKilledTeammate(GetPlayerState()->GetTeam() == AMurphysLawGameMode::TEAM_A);
	}
	else
	{
		// If the player was killed by somebody else
		if (InstigatedBy)
		{
			DeathMessage = FString::Printf(TEXT("%s was killed by %s"), *GetHumanReadableName(), *InstigatedBy->GetHumanReadableName());

			// Increment the number of kills of the other player
			auto OtherPlayerState = Cast<AMurphysLawPlayerState>(InstigatedBy->PlayerState);
			if (OtherPlayerState)
				OtherPlayerState->IncrementNbKills();


			// And increment the number of deaths of the current player
			if (GetPlayerState())
				GetPlayerState()->IncrementNbDeaths();

			if (GameState)
			{
				if (GetPlayerState())
				{
					GameState->PlayerWasKilled(OtherPlayerState->GetTeam() == AMurphysLawGameMode::TEAM_A);
				}
				else
					ShowError("PlayerState is null");
			}
			else
				ShowError("GameState is null");
		}
	}

	AMurphysLawGameMode* GameMode = Cast<AMurphysLawGameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode && DeathMessage != "")
		GameMode->SendDeathMessage(Cast<AMurphysLawPlayerController>(InstigatedBy), DeathMessage);
}

// Returns true if the other actor is an explosive barrel, otherwise false
bool AMurphysLawCharacter::DamageCausedByExplosive(class AActor* OtherActor) const
{
	return Cast<AMurphysLawExplosiveBarrel>(OtherActor) != nullptr;
}

// Returns true if the other actor is an damage zone, false otherwise
bool AMurphysLawCharacter::DamageCausedByDamageZone(class AActor* OtherActor) const
{
	return Cast<AMurphysLawDamageZone>(OtherActor) != nullptr;
}

// Returns true if the controller inflicting the damage is in the same team, false otherwise
bool AMurphysLawCharacter::IsFriendlyFire(class AController* OtherController) const
{
	// If the controller of the actor that hit me is a player controller
	auto OtherPlayerController = Cast<AMurphysLawPlayerController>(OtherController);
	if (OtherPlayerController && OtherPlayerController != GetController())
	{
		// And if we both have a player state
		auto OtherPlayerState = Cast<AMurphysLawPlayerState>(OtherPlayerController->PlayerState);
		if (OtherPlayerState && GetPlayerState())
		{
			// We check if we are on the same team
			return OtherPlayerState->GetTeam() == GetPlayerState()->GetTeam();
		}
	}

	return false;
}

#pragma endregion

// Called when the player press the Reload key
void AMurphysLawCharacter::Reload()
{
	// Checks if we have a weapon equipped
	if (HasWeaponEquipped())
	{
		// If the weapon has been able to reload
		if (GetEquippedWeapon()->Reload())
		{
			SetIsRunning(false);
		}
	}
}

// Reports the reference to the current weapon of the character
AMurphysLawBaseWeapon* AMurphysLawCharacter::GetEquippedWeapon() const
{
	return Inventory->GetWeapon(CurrentWeaponIndex);
}

// Reports if the character has a weapon equipped
bool AMurphysLawCharacter::HasWeaponEquipped() const
{
	return CurrentWeaponIndex != NO_WEAPON_VALUE && GetEquippedWeapon() != nullptr;
}

// Called when the player press a key to change his character's weapon
void AMurphysLawCharacter::EquipWeapon(int32 Index)
{
	// Checks if the index passed in parameter is within the boundaries of our Inventory
	if (Index >= Inventory->NumberOfWeaponInInventory) return;

	// Don't try to re-equip the weapon we already have in hand
	if (Index == CurrentWeaponIndex) return;

	// Allows the character to change weapon even if the current weapon is reloading
	GetEquippedWeapon()->IsReloading = false;

	// Plays a sound when switching weapon if available
	if (SwitchingWeaponSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SwitchingWeaponSound, GetActorLocation());
	}

	// If the server modifies the value of the property, it is sent to the clients automatically
	if (Role == ROLE_Authority)
	{
		Server_ChangeCurrentWeapon_Implementation(Index);
	}
	else
	{
		// If a client wants to modify the value, it has to send a request to the server 
		Server_ChangeCurrentWeapon(Index);
	}
}

// Replicates the current weapon index
bool AMurphysLawCharacter::Server_ChangeCurrentWeapon_Validate(int32 Index) { return true; }
void AMurphysLawCharacter::Server_ChangeCurrentWeapon_Implementation(int32 Index)
{
	int32 OldIndex = CurrentWeaponIndex;
	CurrentWeaponIndex = Index;
	OnRep_CurrentWeaponIndex(OldIndex);
}

// Executed when CurrentWeaponIndex is replicated
void AMurphysLawCharacter::OnRep_CurrentWeaponIndex(int32 OldIndex)
{
	auto OldWeapon = Inventory->GetWeapon(OldIndex);
	auto OldFullMeshWeapon = Inventory->GetFullMeshWeapon(OldIndex);
	auto NewFullMeshWeapon = Inventory->GetFullMeshWeapon(CurrentWeaponIndex);

	// Hide the old weapons
	if (OldWeapon != nullptr) OldWeapon->SetActorHiddenInGame(true);
	if (OldFullMeshWeapon != nullptr) OldFullMeshWeapon->SetActorHiddenInGame(true);

	// And show the new ones
	if (HasWeaponEquipped()) GetEquippedWeapon()->SetActorHiddenInGame(false);
	if (NewFullMeshWeapon != nullptr) NewFullMeshWeapon->SetActorHiddenInGame(false);
}

void AMurphysLawCharacter::ToggleCrouch()
{
	// If the character is not crouched
	if (CanCrouch())
	{
		IsCrouched = true;
		Crouch();

		// Stop the player from running
		SetIsRunning(false);
	}
	else
	{
		IsCrouched = false;
		UnCrouch();
	}
}

// Gets the player state casted to MurphysLawPlayerState
AMurphysLawPlayerState* AMurphysLawCharacter::GetPlayerState() const
{
	return Cast<AMurphysLawPlayerState>(PlayerState);
}

// Returns true if the weapon hold by the character should be reloaded, false otherwise
bool AMurphysLawCharacter::ShouldReload() const
{
	return HasWeaponEquipped()
		&& !GetEquippedWeapon()->IsReloading
		&& GetEquippedWeapon()->GetNumberOfAmmoLeftInMagazine() <= 0
		&& GetEquippedWeapon()->GetNumberOfAmmoLeftInInventory() > 0;
}

void AMurphysLawCharacter::SetMeshTeamColorTint(const MurphysLawTeamColor& TeamColor)
{
	TeamBodyMeshColor = TeamColor.GetPrimary();
	TeamMaskMeshColor = TeamColor.GetDarker();

	if (Role == ROLE_Authority)
	{
		ValidTeamBodyMeshColor = true;
		ValidTeamMaskMeshColor = true;
		ApplyMeshTeamColor();
	}
}

void AMurphysLawCharacter::OnRep_TeamBodyMeshColor()
{
	ValidTeamBodyMeshColor = true;
	ApplyMeshTeamColor();
}

void AMurphysLawCharacter::OnRep_TeamMaskMeshColor()
{
	ValidTeamMaskMeshColor = true;
	ApplyMeshTeamColor();
}

void AMurphysLawCharacter::ApplyMeshTeamColor()
{
	// Put color on meshes
	if (ValidTeamBodyMeshColor && ValidTeamMaskMeshColor)
	{
		/* Get references to mesh material to be able to color them. */
		UMaterialInstanceDynamic* ShaderMeshBody = GetMesh()->CreateDynamicMaterialInstance(0);
		UMaterialInstanceDynamic* ShaderMeshArms = GetMesh1P()->CreateDynamicMaterialInstance(0);
		checkf(ShaderMeshBody != nullptr && ShaderMeshArms != nullptr, TEXT("Unable to find first material on character"));

		ShaderMeshArms->SetVectorParameterValue(MATERIAL_PARAM_TEAM_COLOR_CLOTHES, TeamBodyMeshColor);
		ShaderMeshArms->SetVectorParameterValue(MATERIAL_PARAM_TEAM_COLOR_MASK, TeamMaskMeshColor);
		ShaderMeshBody->SetVectorParameterValue(MATERIAL_PARAM_TEAM_COLOR_CLOTHES, TeamBodyMeshColor);
		ShaderMeshBody->SetVectorParameterValue(MATERIAL_PARAM_TEAM_COLOR_MASK, TeamMaskMeshColor);
	}
}

// Called when the character jumps
void AMurphysLawCharacter::Jump()
{
	// Only jump when the character has enough stamina or isn't already in the air
	if (GetCurrentStaminaLevel() - JumpStaminaDecayAmount <= 0.f
		|| IsInAir()) return;

	// Stop the character from running first
	SetIsRunning(false);

	Super::Jump();

	UpdateStaminaLevel(-JumpStaminaDecayAmount);
}

// Changes the IsRunning state
void AMurphysLawCharacter::SetIsRunning(bool NewValue)
{
	IsRunning = NewValue;

	// If the character starts running, we change its speed
	if (NewValue)
	{
		if (!CanCrouch()) UnCrouch();
		GetCharacterMovement()->MaxWalkSpeed = RUN_SPEED;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = WALK_SPEED;
	}
}

// Reports the current stamina level
float AMurphysLawCharacter::GetCurrentStaminaLevel() const { return CurrentStamina; }

// Reports the maximum stamina level
float AMurphysLawCharacter::GetMaxStaminaLevel() const { return MaxStamina; }

// Function to update the character's stamina
void AMurphysLawCharacter::UpdateStaminaLevel(float StaminaChange)
{
	// Change the stamina level itself
	CurrentStamina += StaminaChange;

	// Make sure the value stays between 0 and its maximum value
	CurrentStamina = UKismetMathLibrary::FClamp(GetCurrentStaminaLevel(), 0.f, GetMaxStaminaLevel());
}

// Determines if an actor is an hittable actor
bool AMurphysLawCharacter::IsHittableActor(AActor* OtherActor)
{
	return OtherActor != nullptr
		&& (OtherActor->IsA<AMurphysLawCharacter>()
			|| OtherActor->IsA<AMurphysLawExplosiveBarrel>());
}

// Called from TeleportTo() when teleport succeeds
void AMurphysLawCharacter::TeleportSucceeded(bool bIsATest)
{
	Super::TeleportSucceeded(bIsATest);

	// To make sure all the weapons are teleported aswell as the character
	Inventory->AttachAllWeaponsToOwner();
}
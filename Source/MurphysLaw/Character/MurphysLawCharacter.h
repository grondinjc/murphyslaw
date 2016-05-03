// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "../Interface/MurphysLawIObjectCollector.h"
#include <MurphysLaw/Settings/Teams/MurphysLawTeamColor.h>
#include "MurphysLawCharacter.generated.h"

class UInputComponent;

UCLASS(config=Game)
class AMurphysLawCharacter : public ACharacter, public IMurphysLawIObjectCollector
{
	GENERATED_BODY()

	const float WALK_SPEED = 600.0f;
	const float RUN_SPEED = 800.0f;

	static const float ROTATION_RATE_HUMAN;
	static const float ROTATION_RATE_BOT;
	static const float FACTOR_HEADSHOT;
	static const float FACTOR_CHESTSHOT;

	static const FName MATERIAL_PARAM_TEAM_COLOR_CLOTHES;
	static const FName MATERIAL_PARAM_TEAM_COLOR_MASK;

	static const FString SOCKET_HEAD;
	static const FString SOCKET_SPINE;

	/** Specifies the value when the character has no weapon in hand */
	const int32 NO_WEAPON_VALUE = -1;

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	/** Scene Component */
	UPROPERTY(VisibleDefaultsOnly, Category = "Scene")
	class USceneComponent* SceneComponent;

	/** Character's name */
	UPROPERTY(VisibleDefaultsOnly, Category = "Name")
	class UWidgetComponent* CharacterNameplate;

	bool IsCharacterAiming = false;
	bool InAir = false;
	float HighestZ;

	/** Hack */
	UPROPERTY(Replicated, ReplicatedUsing=OnRep_TeamBodyMeshColor)
	FColor TeamBodyMeshColor;
	UPROPERTY(Replicated, ReplicatedUsing=OnRep_TeamMaskMeshColor)
	FColor TeamMaskMeshColor;

	UFUNCTION() void OnRep_TeamBodyMeshColor();
	UFUNCTION() void OnRep_TeamMaskMeshColor();

	bool ValidTeamBodyMeshColor;
	bool ValidTeamMaskMeshColor;
	void ApplyMeshTeamColor();

public:
	static const float DefaultAimFactor;

	AMurphysLawCharacter();

	/** Defines a world-space point where an ai should look */
	class AActor* GetFocalPoint() const;

	/** Called when the character is possessed by a new controller */
	void PossessedBy(AController* NewController) override;

	/** Indicates to the server what properties of the object to replicate on the clients */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

	// Provide a color for the meshes of the character
	void SetMeshTeamColorTint(const MurphysLawTeamColor& TeamColor);

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	/** Called when the game starts or when a character spawns */
	void BeginPlay() override;

	// Called when game ends
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Called at every tick in the game */
	void Tick(float DeltaSeconds) override;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

	/** Bearing variable is for the Minimap angle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	float Bearing;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movements")
	bool IsCrouched;

	//Contient le code de crouch
	void ToggleCrouch();

	/** Predicate which determine in which conditions the character can move */
	bool CanPlayerMove();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/** Make the character running forward */
	void Run();

	/** Make the character stop running forward after the release of the shift button */
	void StopRunning();

	/** Aim when the right mouse button is pressed (hold)*/
	void Aim();

	/** Stop aiming on right mouse button release*/
	void StopAiming();

	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }

	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	/** Returns  subobject **/
	FORCEINLINE class UWidgetComponent* GetCharacterNameplate() const { return CharacterNameplate; }

	/** Called from TeleportTo() when teleport succeeds */
	void TeleportSucceeded(bool bIsATest) override;

#pragma region Health and life functions/members
public:
	/** Accessor function for the current amount of health points of the object */
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetCurrentHealthLevel() const;

	/** Accessor function for the maximum amount of health points of the object */
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealthLevel() const;

	/** Reports if the character is dead */
	UFUNCTION(BlueprintPure, Category = "Life")
	bool IsDead() const;

	/** Reports if the character is in the air */
	UFUNCTION(BlueprintPure, Category = "Life")
	bool IsInAir() const;
	void SetIsInAir(bool isInAir);

	/** Makes the character die */
	UFUNCTION(BlueprintCallable, Category = "Life")
	void Die();

	/** Makes the character live again */
	UFUNCTION(BlueprintCallable, Category = "Life")
	void Relive();

protected:
	/** The time before respawning the player, more than 3 seconds for animation to be over */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Life")
	float TimeToRespawn;

	/** The maximum health level of the character */
	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float MaxHealth;

private:
	/** The current health level of the character */
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Health")
	float CurrentHealth;

	/** Bool that shows if the player is dead or alive */
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_Dead, VisibleAnywhere, Category = "Life")
	bool Dead;

#pragma endregion


#pragma region Damage functions
public:

	UFUNCTION(BlueprintCallable, Category = "Damage")
	float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser) override;

protected:
	/** Called from blueprint; Server-side notification that any type of damage were received */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Damage")
	void OnReceiveAnyDamage(float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

private:
	/***/
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser);
#pragma endregion
	
public:
	/**
	Receive an amount of healing from something (environment, pickup ...)
	@param HealAmount This is the added amount of health, and it strictly positive.
	*/
	UFUNCTION(BlueprintCallable, Category = "Health")
	void ReceiveHealAmount(const float HealAmount) override;

	/**
	Receive ammos from something (environment, pickup ...)
	*/
	UFUNCTION(BlueprintCallable, Category = "Ammo")
	void ReceiveAmmo(const int32 NumberOfAmmo) override;

	/**
	Receive gun or ammo from something (environment, pickup ...)
	*/
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void CollectWeapon(class AMurphysLawBaseWeapon* Weapon);

	/** Reports if the character has a weapon equipped */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool HasWeaponEquipped() const;

	/** Accessor function to get the current weapon of the character */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	AMurphysLawBaseWeapon* GetEquippedWeapon() const;

	/**
		Check for collision with the gun with the virtual bullet(s)
		@param Character The character that fired, not someone that was hit
	*/
	void ComputeBulletCollisions();
	
	//Compute the damage based on the distance, the weapon and the bone that was hit
	float GetDeliveredDamage(const FHitResult& CollisionResult) const;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_TransferDamage(class AActor * DamagedActor, const float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, class AActor * DamageCauser);

	UFUNCTION(BlueprintPure, Category = "MiniMap")
	float GetBearing() const { return Bearing; }

	/** Replicates the current weapon index */
	UFUNCTION(Reliable, Server, WithValidation)
	void Server_ChangeCurrentWeapon(int32 Index);

	/** Equip the weapon specified by the Index received in parameter */
	void EquipWeapon(int32 Index);

	/** Fires a projectile. */
	UFUNCTION(BlueprintCallable, Category = "Event")
	void Fire();

	/** Called when the user does a reload */
	void Reload();

	/** Called when the character jumps */
	void Jump() override;

	/** Reports the current stamina level */
	float GetCurrentStaminaLevel() const;

	/** Reports the maximum stamina level */
	float GetMaxStaminaLevel() const;

	/** Function to update the character's stamina */
	void UpdateStaminaLevel(float StaminaChange);

protected: 
	/** Gets the player state casted to MurphysLawPlayerState */
	class AMurphysLawPlayerState* GetPlayerState() const;

	/** The sound that will be played when switching weapon */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	class USoundBase* SwitchingWeaponSound;

	void EquipFirstWeapon();

	/** Keeps the maximum stamina level (overridable in blueprint) */
	UPROPERTY(EditDefaultsOnly, Category = "Stamina")
	float MaxStamina;

private:
	/** The index of the weapon in the inventory that the character is holding */
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_CurrentWeaponIndex)
	int32 CurrentWeaponIndex;

	/** Keeps the running state of the character */
	bool IsRunning;

	/** Keeps the stamina level of the character */
	UPROPERTY(VisibleAnywhere, Category = "Stamina")
	float CurrentStamina;

	/** Represents the amount of stamina lost on each tick when running */
	UPROPERTY(EditDefaultsOnly, Category = "Stamina")
	float RunningStaminaDecayRate;

	/** Represents the amount of stamina lost each time the character jumps */
	UPROPERTY(EditDefaultsOnly, Category = "Stamina")
	float JumpStaminaDecayAmount;

	/** Represents the amount of stamina gained at each tick */
	UPROPERTY(EditDefaultsOnly, Category = "Stamina")
	float StaminaRegenerationRate;

	/** The instance of the inventory of the character */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	class UMurphysLawInventoryComponent* Inventory;

	/** Update the statistics of players involved in the death */
	void UpdateStatsOnKill(class AController* InstigatedBy, class AActor* DamageCauser);
	
	/** Returns true if the other actor is an explosive barrel, false otherwise */
	bool DamageCausedByExplosive(class AActor* OtherActor) const;

	/** Returns true if the other actor is an damage zone, false otherwise */
	bool DamageCausedByDamageZone(class AActor* OtherActor) const;

	/** Returns true if the controller inflicting the damage is in the same team, false otherwise */
	bool IsFriendlyFire(class AController* OtherController) const;

	/** Returns true if the weapon hold by the character should be reloaded, false otherwise */
	bool ShouldReload() const;

	/** Configure character movement with human or bot specific settings */
	void ConfigureMovement(const bool ConfigureForBot);

	/** Changes the IsRunning state */
	void SetIsRunning(bool NewValue);

	/** Executed when CurrentWeaponIndex is replicated */
	UFUNCTION()
	void OnRep_CurrentWeaponIndex(int32 OldIndex);

	/** Executed when the Dead variable is replicated */
	UFUNCTION()
	void OnRep_Dead();

	/** Determines if an actor is an hittable actor */
	static bool IsHittableActor(AActor* OtherActor);
};


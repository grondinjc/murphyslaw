// Fill out your copyright notice in the Description page of Project Settings.

#include "MurphysLaw.h"
#include "MurphysLawBaseWeapon.h"
#include "MurphysLawProjectile.h"
#include "../Character/MurphysLawCharacter.h"

DEFINE_LOG_CATEGORY(ML_BaseWeapon);

// Sets default values
AMurphysLawBaseWeapon::AMurphysLawBaseWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComp"));
	RootComponent = CollisionComp;

	WeaponStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponStaticMesh->AttachTo(RootComponent);

	// Default offset from the weapon location for projectiles to spawn
	WeaponOffset = FVector(100.0f, 30.0f, 10.0f);

	// Sets the default magazine data
	MaximumNumberOfAmmoInMagazine = 30; // The size of a magazine
	MaximumNumberOfAmmoInInventory = 120; // The maximum number of ammo the character can have for this gun
	StartingNumberOfAmmoInInventory = 90; // The number of ammo the character starts with

	// Allow blueprint to specify which sound they want
	// All sound resources should be checked in BeginPlay to display warning messages
	Sounds.Fire = nullptr;
	Sounds.DryWeapon = nullptr;
	Sounds.Reload = nullptr;

	IsReloading = false;

	// Set the default fragment properties (overridden by subclasses)
	MaxTraveledDistanceOfBullet = 150 * 100.f; // (in centimeters)
	FragmentBruteDamageAmount = 150.f;
	NumberOfEmittedFragments = 1;
	MaxFragmentDeviationAngle = 0.5f;
	DamageDistanceAmplicator = 10.f;

	AimFactor = AMurphysLawCharacter::DefaultAimFactor;

	WeaponName = TEXT("-- No Name --");
}

// Called when the game starts or when spawned
void AMurphysLawBaseWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	// Debug messages
	if (Sounds.DryWeapon == nullptr) ShowWarning(FString::Printf(TEXT("[%s] - Resource 'Sounds.DryWeapon' is not set"), *GetName()));
	if (Sounds.Fire == nullptr) ShowWarning(FString::Printf(TEXT("[%s] - Resource 'Sounds.Fire' is not set"), *GetName()));
	if (Sounds.Reload == nullptr) ShowWarning(FString::Printf(TEXT("[%s] - Resource 'Sounds.Reload' is not set"), *GetName()));

	// Evaluate invariants
	verifyf(DamageDistanceAmplicator >= 1, TEXT("The gun 'DamageDistanceAmplicator' needs to be >= 1"));
	
	// check if the starting amount of ammo is over the inventory maximum and set it to the correct amount
	NumberOfAmmoLeftInInventory = FMath::Min(StartingNumberOfAmmoInInventory, MaximumNumberOfAmmoInInventory);

	// Reloads the gun as it starts empty
	IsReloading = true;
	Reload_Implementation();
}

// Called when the game ends
void AMurphysLawBaseWeapon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

// Try to fire the weapon and returns whether it worked or not
bool AMurphysLawBaseWeapon::Fire(AMurphysLawCharacter* Character)
{
	// Check if the weapon has ammo
	if (CanFire())
	{
		// try and fire a projectile
		if (ProjectileClass != nullptr)
		{
			const FRotator SpawnRotation = Character->GetControlRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = Character->GetActorLocation() + SpawnRotation.RotateVector(WeaponOffset);

			UWorld* const World = GetWorld();
			if (World != nullptr)
			{
				// spawn the projectile at the muzzle
				World->SpawnActor<AMurphysLawProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
			}
		}

		// try and play the sound if specified
		if (Sounds.Fire != nullptr)
		{
			UGameplayStatics::PlaySoundAtLocation(this, Sounds.Fire, GetActorLocation());
		}

		// Decrement the number of ammo left in the magazine
		--NumberOfAmmoLeftInMagazine;

		return true;
	}
	else if (!IsReloading)
	{
		// If the gun is not reloading which means the gun is out of ammo.
				
		// Try and play the Dry Weapon sound
		if (Sounds.DryWeapon != nullptr)
		{
			UGameplayStatics::PlaySoundAtLocation(this, Sounds.DryWeapon, GetActorLocation());
		}

		// If there are still ammos in the Inventory, show the reload hint to the player
		if (NumberOfAmmoLeftInInventory > 0)
		{
			ShowMessage("Press R to reload", FColor::Cyan);
		}
	}

	return false;
}

// Tries to reload the weapon and tells the character if it has reloaded or not
// so the character can play an animation
bool AMurphysLawBaseWeapon::Reload()
{
	// check if we can reload the weapon
	if (CanReload())
	{
		// Changes the state of the weapon to 'Reloading'
		IsReloading = true;
		return true;
	}

	return false;
}

/** Reinitializes a the ammos of the gun to default */
void AMurphysLawBaseWeapon::Reinitialize()
{
	// Checks if the starting amount of ammo is over the inventory's maximum
	//	and sets the amount to max if it's higher
	NumberOfAmmoLeftInInventory = FMath::Min(StartingNumberOfAmmoInInventory, MaximumNumberOfAmmoInInventory);

	// When a weapon is reinitialized, it is hidden by default
	SetActorHiddenInGame(true);

	// Reloads the gun as it starts empty
	NumberOfAmmoLeftInMagazine = 0;
	IsReloading = true;
	Reload_Implementation();
}

// Deals with all the arithmetic behind the reloading
void AMurphysLawBaseWeapon::Reload_Implementation()
{
	// If the reloading has been cancelled, skip the implementation
	if (!IsReloading) return;

	// Calculate how many ammo we can put in our magazine right now
	int32 NumberOfAmmoToReload = MaximumNumberOfAmmoInMagazine - NumberOfAmmoLeftInMagazine;

	// if there's less ammo left in our inventory than what we can use we take it all
	if (NumberOfAmmoLeftInInventory < NumberOfAmmoToReload)
	{
		NumberOfAmmoToReload = NumberOfAmmoLeftInInventory;
	}

	// Swap the ammos from the inventory to the weapon
	NumberOfAmmoLeftInMagazine += NumberOfAmmoToReload;
	NumberOfAmmoLeftInInventory -= NumberOfAmmoToReload;
	
	// Indicates the reload is done
	IsReloading = false;
}

void AMurphysLawBaseWeapon::PlayReloadSound()
{
	// Try and play the Reload sound
	// If the weapon isn't reloading anymore, we don't play the sound
	if (Sounds.Reload != nullptr && IsReloading)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Sounds.Reload, GetActorLocation());
	}
}

// Reports the number of ammo left in the weapon
int32 AMurphysLawBaseWeapon::GetNumberOfAmmoLeftInMagazine() const
{
	return NumberOfAmmoLeftInMagazine;
}

/** Sets the number of ammo left in the magazine */
void AMurphysLawBaseWeapon::SetNumberOfAmmoLeftInMagazine(int32 NumberOfAmmo)
{
	NumberOfAmmoLeftInMagazine = NumberOfAmmo;
}

// Reports the maximum nomber of ammo that can be carried in the inventory
int32 AMurphysLawBaseWeapon::GetMaxAmmoInInventory() const
{
	return MaximumNumberOfAmmoInInventory;
}

// Reports the number of ammo left in the inventory
int32 AMurphysLawBaseWeapon::GetNumberOfAmmoLeftInInventory() const
{
	return NumberOfAmmoLeftInInventory;
}

/** Sets the number of ammo left in the inventory */
void AMurphysLawBaseWeapon::SetNumberOfAmmoLeftInInventory(int32 NumberOfAmmo)
{
	NumberOfAmmoLeftInInventory = NumberOfAmmo;
}

EWeaponTypes AMurphysLawBaseWeapon::GetWeaponType() const
{
	return WeaponType;
}

// Updates the number of ammo currently in the inventory
void AMurphysLawBaseWeapon::AddAmmoInInventory(int32 AmountOfAmmo)
{
	if (AmountOfAmmo < 0)
	{
		return;
	}
	NumberOfAmmoLeftInInventory = FMath::Min(GetNumberOfAmmoLeftInInventory() + AmountOfAmmo, MaximumNumberOfAmmoInInventory);
}

// Reports if the character can fire the weapon
bool AMurphysLawBaseWeapon::CanFire() const
{
	return NumberOfAmmoLeftInMagazine > 0 && !IsReloading;
}

// Reports if the character can reload the weapon
bool AMurphysLawBaseWeapon::CanReload() const
{
	return GetNumberOfAmmoLeftInMagazine() < MaximumNumberOfAmmoInMagazine && GetNumberOfAmmoLeftInInventory() > 0 && !IsReloading;
}

#pragma region Bullet info

/** Represents the maximum traveled distance of a bullet */
float AMurphysLawBaseWeapon::GetMaxTravelDistanceOfBullet() const { return MaxTraveledDistanceOfBullet; }

/** Represents the theoretical damage of a bullet colliding with a null distance (value of zero) */
float AMurphysLawBaseWeapon::GetFragmentBruteDamageAmount() const { return FragmentBruteDamageAmount; }

/** Represents the maximum traveled distance of a bullet */
uint8 AMurphysLawBaseWeapon::GetNumberOfEmittedFragments() const { return NumberOfEmittedFragments; }

/** Represents the maximum traveled distance of a bullet */
float AMurphysLawBaseWeapon::GetMaxFragmentDeviationAngle(bool IsCharacterAiming) const { return IsCharacterAiming ? MaxFragmentDeviationAngleOnAiming : MaxFragmentDeviationAngle; }

/** Represents the maximum traveled distance of a bullet */
float AMurphysLawBaseWeapon::GetDamageDistanceAmplicator() const { return DamageDistanceAmplicator; }
		
float AMurphysLawBaseWeapon::ComputeCollisionDamage(const float ImpactDistance) const
{
	return FragmentBruteDamageAmount * FMath::Square(1.0f - (ImpactDistance / MaxTraveledDistanceOfBullet)) / DamageDistanceAmplicator;
}

#pragma endregion

// Tells whether the weapon is of same type or not
bool AMurphysLawBaseWeapon::IsOfSameType(AMurphysLawBaseWeapon* OtherWeapon) const
{
	return GetWeaponType() == OtherWeapon->GetWeaponType();
}
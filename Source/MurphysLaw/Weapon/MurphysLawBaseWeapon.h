// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "MurphysLawBaseWeapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponTypes : uint8
{
	EPistol			UMETA(DisplayName = "Pistol"),
	ERifle			UMETA(DisplayName = "Rifle"),
	EShotgun		UMETA(DisplayName = "Shotgun")
};

USTRUCT()
struct FWeaponSounds
{
	GENERATED_USTRUCT_BODY();

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class USoundBase* Fire;

	/** Sound to play each time we fire and we're out of ammo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class USoundBase* DryWeapon;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class USoundBase* Reload;
};

DECLARE_LOG_CATEGORY_EXTERN(ML_BaseWeapon, Log, All);

UCLASS(abstract)
class MURPHYSLAW_API AMurphysLawBaseWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMurphysLawBaseWeapon();

	// Called when the game starts or when spawned
	void BeginPlay() override;

	// Called when the game ends
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Weapon muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	FVector WeaponOffset;

	/** Scope magnifying factor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Aim)
	float AimFactor;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class AMurphysLawProjectile> ProjectileClass;

	/** Stores all the sounds for the weapon */
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	FWeaponSounds Sounds;

	/** Accessors function for the number of ammo left in the magazine */
	UFUNCTION(BlueprintPure, Category = "Ammo")
	int32 GetNumberOfAmmoLeftInMagazine() const;

	/** Sets the number of ammo left in the magazine */
	void SetNumberOfAmmoLeftInMagazine(int32 NumberOfAmmo);

	/** Accessors function for the number of ammo left in the inventory */
	UFUNCTION(BlueprintPure, Category = "Ammo")
	int32 GetMaxAmmoInInventory() const;

	/** Accessors function for the number of ammo left in the inventory */
	UFUNCTION(BlueprintPure, Category = "Ammo")
	int32 GetNumberOfAmmoLeftInInventory() const;

	/** Sets the number of ammo left in the inventory */
	void SetNumberOfAmmoLeftInInventory(int32 NumberOfAmmo);

	/** Accessors function for the number of ammo left in the inventory */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	EWeaponTypes GetWeaponType() const;

	/** Accessors function for the number of ammo left in the inventory */
	UFUNCTION(BlueprintCallable, Category = "Ammo")
	void AddAmmoInInventory(int32 NewNumberOfAmmo);
	
	/** Function called when the character wants to fire the weapon */
	bool Fire(class AMurphysLawCharacter* Character);

	/** Function called when the character wants to reload the weapon */
	bool Reload();
	
	/** Reinitializes a the ammos of the gun to default */
	void Reinitialize();

	/** Function that actually makes calculation to remove the good number of ammo from the Inventory and put it in the gun */
	/** 
		For instance, if we need to make a gun that discards a whole magazine on reload, we can override this method in a child
		class (let's say MurphysLawWeaponDiscardMagazine) and make the new implementation for the calculation. Then we can create 
		a new Blueprint class that will be a child of MurphysLawBaseWeaponDiscardMagazine.
	*/
	UFUNCTION(BlueprintCallable, Category = "Ammo")
	virtual void Reload_Implementation();

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	virtual void PlayReloadSound();

	/** Tells whether the weapon is of same type or not */
	bool IsOfSameType(AMurphysLawBaseWeapon* OtherWeapon) const;

	/** Tells whether the gun is reloading right now or not */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Ammo")
	bool IsReloading;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Collision")
	class UBoxComponent* CollisionComp;

	/** Reports the weapon name */
	FORCEINLINE FString GetWeaponName() const { return WeaponName; }

	FORCEINLINE class UStaticMeshComponent* GetWeaponStaticMesh() { return WeaponStaticMesh; }

protected:
	/** Represents the number of ammo left in the Magazine (that can be shot now) */
	UPROPERTY(VisibleAnywhere, Category = "Ammo")
	int32 NumberOfAmmoLeftInMagazine;

	/** Represents the maximum number of ammo a Magazine can hold */
	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 MaximumNumberOfAmmoInMagazine;

	/** Represents the number of ammo left in the Inventory for this weapon (that can be loaded in the weapon now) */
	UPROPERTY(VisibleAnywhere, Category = "Ammo")
	int32 NumberOfAmmoLeftInInventory;

	/** Represents the maximum number of number of ammo the Inventory can hold */
	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 MaximumNumberOfAmmoInInventory;

	/** Represents the starting number of ammo in the Inventory for the weapon */
	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 StartingNumberOfAmmoInInventory;

#pragma region Bullet info
public:
	/** Represents the maximum traveled distance of a bullet */
	UFUNCTION(BlueprintPure, Category = "Fragments")
	float GetMaxTravelDistanceOfBullet() const;

	/** Represents the theoretical damage of a bullet colliding with a null distance (value of zero) */
	UFUNCTION(BlueprintPure, Category = "Fragments")
	float GetFragmentBruteDamageAmount() const;

	/** Represents the fragments sent when shooting */
	UFUNCTION(BlueprintPure, Category = "Fragments")
	uint8 GetNumberOfEmittedFragments() const;

	/** Represents the deviation of a fired bullet from perfect direction */
	UFUNCTION(BlueprintPure, Category = "Fragments")
	float GetMaxFragmentDeviationAngle(bool IsCharacterAiming) const;

	/** Represents the attenuation factor of damage over distance */
	UFUNCTION(BlueprintPure, Category = "Fragments")
	float GetDamageDistanceAmplicator() const;

	/** Evaluate the damage caused by a bullet based on the impact distance (in cm) */
	UFUNCTION(Category = "Fragments")
	float ComputeCollisionDamage(const float ImpactDistance) const;

	/** Represents the maximum angle deviation of the emitted fragments when firing (in degrees) */
	UPROPERTY(EditDefaultsOnly, Category = "Fragments")
	float MaxFragmentDeviationAngle;

	/** Represents the maximum angle deviation of the emitted fragments when firing (in degrees) */
	UPROPERTY(EditDefaultsOnly, Category = "Fragments")
	float MaxFragmentDeviationAngleOnAiming;

protected:
	/** Represents the maximum travel distance that a bullet will reach for collision detection (in cm) */
	UPROPERTY(EditDefaultsOnly, Category = "Fragments")
	float MaxTraveledDistanceOfBullet;

	/** Represents the theoretical damage of a bullet colliding with a null distance (value of zero) */
	UPROPERTY(EditDefaultsOnly, Category = "Fragments")
	float FragmentBruteDamageAmount;

	/** Represents the number of fragments emitted when shooting */
	UPROPERTY(EditDefaultsOnly, Category = "Fragments")
	uint8 NumberOfEmittedFragments;

	/**
	Represents the distance factor at which damage over distance reduces.
	Value needs to be greater or equal to 1.
	A value of 1 does not amplifies the distance.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Fragments")
	float DamageDistanceAmplicator;

#pragma endregion

protected:
	bool CanReload() const;

	bool CanFire() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	TEnumAsByte<EWeaponTypes> WeaponType;

	/** Keeps the weapon name */
	UPROPERTY(EditDefaultsOnly, Category = "Name")
	FString WeaponName;

private:
	/** Weapon mesh: 1st person view (seen only by the character holding it) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class UStaticMeshComponent* WeaponStaticMesh;
};

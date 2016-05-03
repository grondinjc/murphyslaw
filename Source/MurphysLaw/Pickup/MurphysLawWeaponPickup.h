// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Pickup/MurphysLawPickup.h"
#include "MurphysLawWeaponPickup.generated.h"

/**
 * 
 */
UCLASS()
class MURPHYSLAW_API AMurphysLawWeaponPickup : public AMurphysLawPickup
{
	GENERATED_BODY()

public:
	/** Called when the actor is spawned */
	void BeginPlay() override;
	
	/** Define the ammo transfer */
	void ExecuteCollectorInterraction(class AMurphysLawCharacter* Character) override;

	/** Keeps a reference on the weapon the pickup created */
	UPROPERTY(VisibleAnywhere, Category = "Pickup")
	class AMurphysLawBaseWeapon* WeaponInstance;

	/** The type of the weapon that the pickup is holding */
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	TSubclassOf<class AMurphysLawBaseWeapon> WeaponType;

	/** Changes the pickup visibility */
	void SetPickupVisible(bool IsVisible) override;
	
protected:
	/** Returns the weapon instance of the pickup */
	UFUNCTION(BlueprintPure, Category = "WeaponInstance")
	class AMurphysLawBaseWeapon* GetWeaponInstance() const;	
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Pickup/MurphysLawPickup.h"
#include "MurphysLawAmmoPickup.generated.h"

/**
 * 
 */
UCLASS()
class MURPHYSLAW_API AMurphysLawAmmoPickup : public AMurphysLawPickup
{
	GENERATED_BODY()

protected:

	// Define the health transfer
	void ExecuteCollectorInterraction(AMurphysLawCharacter* Character) override;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	int32 AmmoBonus;
};

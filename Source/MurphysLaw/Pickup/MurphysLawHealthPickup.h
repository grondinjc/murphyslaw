// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MurphysLawPickup.h"
#include "MurphysLawHealthPickup.generated.h"

/**
 * 
 */
UCLASS()
class MURPHYSLAW_API AMurphysLawHealthPickup : public AMurphysLawPickup
{
	GENERATED_BODY()

protected:

	// Define the health transfer
	void ExecuteCollectorInterraction(AMurphysLawCharacter* Character) override;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	float LifeBonus;
};

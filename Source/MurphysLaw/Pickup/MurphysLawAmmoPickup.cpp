// Fill out your copyright notice in the Description page of Project Settings.

#include "MurphysLaw.h"
#include "../Interface/MurphysLawIObjectCollector.h"
#include "MurphysLawAmmoPickup.h"

void AMurphysLawAmmoPickup::ExecuteCollectorInterraction(AMurphysLawCharacter* Character)
{
	Super::ExecuteCollectorInterraction(Character);

	// add ammos for the current gun
	Character->ReceiveAmmo(AmmoBonus);
}


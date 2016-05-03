// Fill out your copyright notice in the Description page of Project Settings.

#include "MurphysLaw.h"
#include "../Interface/MurphysLawIObjectCollector.h"
#include "MurphysLawHealthPickup.h"

void AMurphysLawHealthPickup::ExecuteCollectorInterraction(AMurphysLawCharacter* Character)
{
	Super::ExecuteCollectorInterraction(Character);
	
	if (Role == ROLE_Authority)
	{
		// transfer health points
		Character->ReceiveHealAmount(LifeBonus);
	}
}


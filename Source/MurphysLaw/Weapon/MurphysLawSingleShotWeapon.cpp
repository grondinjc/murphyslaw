// Fill out your copyright notice in the Description page of Project Settings.

#include "MurphysLaw.h"
#include "../Character/MurphysLawCharacter.h"
#include "MurphysLawSingleShotWeapon.h"

AMurphysLawSingleShotWeapon::AMurphysLawSingleShotWeapon()
{
	// Set the default fragment properties for a multi-shot gun
	MaxTraveledDistanceOfBullet = 150 * 100.f; // (in centimeters)
	FragmentBruteDamageAmount = 150.f;
	NumberOfEmittedFragments = 1;
	MaxFragmentDeviationAngle = 1.f;
	DamageDistanceAmplicator = 10.f;
}

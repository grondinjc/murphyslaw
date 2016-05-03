// Fill out your copyright notice in the Description page of Project Settings.

#include "MurphysLaw.h"
#include "MurphysLawMultiShotWeapon.h"

AMurphysLawMultiShotWeapon::AMurphysLawMultiShotWeapon()
{
	// Set the default fragment properties for a multi-shot gun
	MaxTraveledDistanceOfBullet = 25 * 100.f; // (in centimeters)
	FragmentBruteDamageAmount = 100.f;
	NumberOfEmittedFragments = 12;
	MaxFragmentDeviationAngle = 20.f;
	DamageDistanceAmplicator = 10.f;
}

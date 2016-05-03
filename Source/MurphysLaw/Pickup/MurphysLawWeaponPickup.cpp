// Fill out your copyright notice in the Description page of Project Settings.

#include "MurphysLaw.h"
#include "../Character/MurphysLawCharacter.h"
#include "../Interface/MurphysLawIObjectCollector.h"
#include "MurphysLawWeaponPickup.h"
#include "../Weapon/MurphysLawBaseWeapon.h"

// Called when the actor is spawned
void AMurphysLawWeaponPickup::BeginPlay()
{
	Super::BeginPlay();

	// The WeaponPickup must have a weapon type set
	checkf(WeaponType != nullptr, TEXT("No weapon type is set for this WeaponPickup"));

	// Prepare the ActorSpawnParameters for all the weapons
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = Instigator;

	// If the Weapon Type has been set, we spawn a weapon of that type
	auto SpawnedWeapon = GetWorld()->SpawnActor<AMurphysLawBaseWeapon>(WeaponType, SpawnParams);

	// If the weapon was spawned successfully, we store it in our inventory
	if (SpawnedWeapon != nullptr)
	{
		WeaponInstance = SpawnedWeapon;
		WeaponInstance->SetActorLocation(GetActorLocation());

		// When the server spawns the weapon, he starts its movement replication and repliaction in general
		if (Role == ROLE_Authority)
		{
			WeaponInstance->SetReplicates(true);
			WeaponInstance->SetReplicateMovement(true);
		}
		else
		{
			// If the client spawns the pickup, he hides it beceause the server will replicate his
			WeaponInstance->SetActorHiddenInGame(true);
		}
	}

	// The WeaponPickup must have a weapon instance set
	checkf(WeaponInstance != nullptr, TEXT("No weapon instance is set for this WeaponPickup"));

}

// Define the ammo transfer
void AMurphysLawWeaponPickup::ExecuteCollectorInterraction(AMurphysLawCharacter* Character)
{
	Super::ExecuteCollectorInterraction(Character);

	// add ammos for the current gun
	Character->CollectWeapon(WeaponInstance);
}

// Returns the weapon instance of the pickup
AMurphysLawBaseWeapon* AMurphysLawWeaponPickup::GetWeaponInstance() const
{
	return WeaponInstance;
}

// Changes the pickup visibility
void AMurphysLawWeaponPickup::SetPickupVisible(bool IsVisible)
{
	Super::SetPickupVisible(IsVisible);

	// Only the server changes the weapon visibility as it's replicated
	if (Role == ROLE_Authority)
	{
		// Change the weapon instance visibility according to the value received in parameter
		WeaponInstance->SetActorHiddenInGame(!IsVisible);
	}
}
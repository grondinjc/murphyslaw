// Fill out your copyright notice in the Description page of Project Settings.

#include "MurphysLaw.h"
#include "MurphysLawInventoryComponent.h"
#include "../Character/MurphysLawCharacter.h"
#include "../Weapon/MurphysLawBaseWeapon.h"

// Sets default values for this component's properties
UMurphysLawInventoryComponent::UMurphysLawInventoryComponent()
{
	bWantsBeginPlay = true;

	// Sets the default number of weapon in the inventory
	Weapons.SetNum(NumberOfWeaponInInventory, false);
	WeaponsFullMesh.SetNum(NumberOfWeaponInInventory, false);
	WeaponTypes.SetNum(NumberOfWeaponInInventory, false);

	Owner = nullptr;
}

// Called when the game starts
void UMurphysLawInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// Gets the owner of the Inventory (to spawn the weapons)
	Owner = Cast<AMurphysLawCharacter>(GetOwner());

	checkf(Owner != nullptr, TEXT("Inventory Component has no owner"));

	// Create all the weapons the character starts the game with
	for (int32 i = 0; i < NumberOfWeaponInInventory; ++i)
	{
		if (WeaponTypes[i] == nullptr)
		{
			continue;
		}

		AMurphysLawBaseWeapon *ArmsWeapon = nullptr, *FullMeshWeapon = nullptr;

		// Spawn the weapon
		SpawnWeapon(WeaponTypes[i], ArmsWeapon, FullMeshWeapon);

		// If the weapon was spawned successfully, we store it in our inventory
		if (ArmsWeapon != nullptr) Weapons[i] = ArmsWeapon;
		if (FullMeshWeapon != nullptr) WeaponsFullMesh[i] = FullMeshWeapon;
	}
}

// Called when the game ends
void UMurphysLawInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	checkf(Weapons.Num() == WeaponsFullMesh.Num(), TEXT("Different number of weapons"));

	// Destroy all the weapons because the player is dead
	for (int i = 0; i < Weapons.Num(); ++i)
	{
		if (Weapons[i] != nullptr) Weapons[i]->Destroy();
		if (WeaponsFullMesh[i] != nullptr) WeaponsFullMesh[i]->Destroy();
	}
}

// Accessor function for the weapons in the inventory
AMurphysLawBaseWeapon* UMurphysLawInventoryComponent::GetWeapon(int32 Index)
{
	// Checks if the Index of the weapon to get is valid
	if (Index < 0 || Index >= NumberOfWeaponInInventory)
	{
		return nullptr;
	}

	return Weapons[Index];
}

/** Accessor function for the weapons of the full mesh in the inventory */
AMurphysLawBaseWeapon* UMurphysLawInventoryComponent::GetFullMeshWeapon(int32 Index)
{
	// Checks if the Index of the weapon to get is valid
	if (Index < 0 || Index >= NumberOfWeaponInInventory)
	{
		return nullptr;
	}

	return WeaponsFullMesh[Index];
}

// Receive gun or ammo from something (environment, pickup ...)
void UMurphysLawInventoryComponent::CollectWeapon(AMurphysLawBaseWeapon* NewWeapon)
{
	bool IsNewWeapon = true;
	for (auto Weapon : Weapons)
	{
		if (Weapon->IsOfSameType(NewWeapon))
		{
			Weapon->AddAmmoInInventory(NewWeapon->GetNumberOfAmmoLeftInInventory() + NewWeapon->GetNumberOfAmmoLeftInMagazine());
			IsNewWeapon = false;
		}
	}

	if (IsNewWeapon)
	{
		TakeWeapon(NewWeapon);
	}
}

// Take a new weapon
void UMurphysLawInventoryComponent::TakeWeapon(AMurphysLawBaseWeapon* NewWeapon)
{
	AMurphysLawBaseWeapon *ArmsWeapon = nullptr, *FullMeshWeapon = nullptr;

	// If the Weapon Type has been set, we spawn a weapon of that type
	SpawnWeapon(NewWeapon->GetClass(), ArmsWeapon, FullMeshWeapon);

	// If the weapon was spawned successfully, we store it in our inventory
	if (ArmsWeapon != nullptr)
	{
		// Set the inventory data
		Weapons.SetNum(NumberOfWeaponInInventory + 1, false);
		WeaponsFullMesh.SetNum(NumberOfWeaponInInventory + 1, false);

		ArmsWeapon->SetNumberOfAmmoLeftInMagazine(NewWeapon->GetNumberOfAmmoLeftInMagazine());
		ArmsWeapon->SetNumberOfAmmoLeftInInventory(NewWeapon->GetNumberOfAmmoLeftInInventory());
		Weapons[NumberOfWeaponInInventory] = ArmsWeapon;
		WeaponsFullMesh[NumberOfWeaponInInventory] = FullMeshWeapon;

		NumberOfWeaponInInventory++;
	}
	else
	{
		ShowError(TEXT("Unable to take the new weapon"));
	}
}

// Function that spawns all the weapons the inventory holds 
void UMurphysLawInventoryComponent::SpawnWeapon(TSubclassOf<AMurphysLawBaseWeapon> WeaponType, AMurphysLawBaseWeapon*& ArmsWeapon, AMurphysLawBaseWeapon*& FullMeshWeapon) const
{
	ArmsWeapon = nullptr;
	FullMeshWeapon = nullptr;

	// If the WeaponType is valid, we spawn the weapon
	if (WeaponType != nullptr)
	{
		// Sets the information to spawn a weapon
		FActorSpawnParameters WeaponSpawnParams;
		WeaponSpawnParams.Owner = Owner;
		WeaponSpawnParams.Instigator = Owner->Instigator;

		ArmsWeapon = GetWorld()->SpawnActor<AMurphysLawBaseWeapon>(WeaponType, WeaponSpawnParams);
		FullMeshWeapon = GetWorld()->SpawnActor<AMurphysLawBaseWeapon>(WeaponType, WeaponSpawnParams);

		// If the weapon was spawned successfully, we set the common properties
		if (FullMeshWeapon != nullptr)
		{
			FullMeshWeapon->AttachRootComponentTo(Owner->GetMesh(), "GripPoint", EAttachLocation::SnapToTarget);
			FullMeshWeapon->SetActorHiddenInGame(true);
			FullMeshWeapon->GetWeaponStaticMesh()->SetOwnerNoSee(true);
		}

		// If the weapon was spawned successfully, we set the common properties
		if (ArmsWeapon != nullptr)
		{
			ArmsWeapon->AttachRootComponentTo(Owner->GetMesh1P(), "GripPoint", EAttachLocation::SnapToTarget);
			ArmsWeapon->SetActorHiddenInGame(true);
			ArmsWeapon->GetWeaponStaticMesh()->SetOnlyOwnerSee(true);
		}
	}
}

// Reinitializes a character's inventory to default
void UMurphysLawInventoryComponent::Reinitialize()
{
	checkf(Weapons.Num() == WeaponsFullMesh.Num(), TEXT("Different number of weapons"));	

	// Removes the collected weapons during the last "life"
	while (NumberOfWeaponInInventory > NB_WEAPON_AT_START)
	{
		NumberOfWeaponInInventory--;

		Weapons[NumberOfWeaponInInventory]->Destroy();
		Weapons[NumberOfWeaponInInventory] = nullptr;
		Weapons.SetNum(NumberOfWeaponInInventory, false);

		WeaponsFullMesh[NumberOfWeaponInInventory]->Destroy();
		WeaponsFullMesh[NumberOfWeaponInInventory] = nullptr;
		WeaponsFullMesh.SetNum(NumberOfWeaponInInventory, false);
	}

	// Resets the guns still in inventory
	for (int i = 0; i < NB_WEAPON_AT_START; i++)
	{
		Weapons[i]->Reinitialize();
		WeaponsFullMesh[i]->Reinitialize();
	}
}

// Attaches all weapons to owner
void UMurphysLawInventoryComponent::AttachAllWeaponsToOwner()
{
	checkf(Weapons.Num() == WeaponsFullMesh.Num(), TEXT("Different number of weapons"));

	for (int i = 0; i < Weapons.Num(); ++i)
	{
		Weapons[i]->GetRootComponent()->DetachFromParent();
		Weapons[i]->AttachRootComponentTo(Owner->GetMesh1P(), "GripPoint", EAttachLocation::SnapToTarget);
		WeaponsFullMesh[i]->GetRootComponent()->DetachFromParent();
		WeaponsFullMesh[i]->AttachRootComponentTo(Owner->GetMesh(), "GripPoint", EAttachLocation::SnapToTarget);
	}
}
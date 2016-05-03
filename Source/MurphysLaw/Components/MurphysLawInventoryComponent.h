// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "MurphysLawInventoryComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MURPHYSLAW_API UMurphysLawInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	/** Sets default values for this component's properties */
	UMurphysLawInventoryComponent();

	/** Called when the game starts */
	void BeginPlay() override;

	/** Called when the game ends */
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Receive gun or ammo from something (environment, pickup ...) */
	void CollectWeapon(class AMurphysLawBaseWeapon* NewWeapon);

	/** Specifies the number of Inventory slots a character has */
	const int32 NB_WEAPON_AT_START = 2;
	int32 NumberOfWeaponInInventory = NB_WEAPON_AT_START;

	/** Accessor function for the weapons in the inventory */
	class AMurphysLawBaseWeapon* GetWeapon(int32 Index);

	/** Accessor function for the weapons of the full mesh in the inventory */
	class AMurphysLawBaseWeapon* GetFullMeshWeapon(int32 Index);

	/** Reinitializes a character's inventory to default */
	void Reinitialize();

	/** Attaches all weapons to owner */
	void AttachAllWeaponsToOwner();

protected:
	/** The types of weapon the character starts the game with */
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TArray<TSubclassOf<class AMurphysLawBaseWeapon>> WeaponTypes;

private:
	/** Keeps the weapons the character can hold */
	TArray<class AMurphysLawBaseWeapon*> Weapons;

	/** Keeps the weapons the character can hold */
	TArray<class AMurphysLawBaseWeapon*> WeaponsFullMesh;

	/** The reference on the component's owner */
	class AMurphysLawCharacter* Owner;

	/** Take a new weapon */
	void TakeWeapon(class AMurphysLawBaseWeapon* Weapon);

	/** Function that spawns all the weapons the inventory holds */
	void SpawnWeapon(TSubclassOf<class AMurphysLawBaseWeapon> WeaponType, class AMurphysLawBaseWeapon*& ArmsWeapon, class AMurphysLawBaseWeapon*& FullMeshWeapon) const;
};

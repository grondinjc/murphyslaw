// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "../Character/MurphysLawCharacter.h"
#include "MurphysLawPickup.generated.h"

USTRUCT()
struct FPickupSounds
{
	GENERATED_USTRUCT_BODY();

	/** Sound to play each time the pickup is collected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	class USoundBase* CollectSound;
};


UCLASS()
class MURPHYSLAW_API AMurphysLawPickup : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	float RespawnTime;

	// Code for replication
	//UPROPERTY(Transient, ReplicatedUsing = OnRep_ActiveRepere)
	bool bVisiblePickup;

	// Defines the interaction of the pickup with the actor
	virtual void ExecuteCollectorInterraction(AMurphysLawCharacter* Character);

private:
	/** Contact sphere */
	UPROPERTY(VisibleDefaultsOnly, Category = "Pickup")
	class USphereComponent* SphereContact;

	/** Handle for efficient management of RespawnPickup timer */
	FTimerHandle TimerHandle_RespawnPickup;

	/** Stores all the sounds for the weapon */
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	FPickupSounds Sounds;

	/** Event handler when an actors enters the collision zone */
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void OnBeginOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
public:	
	// Sets default values for this actor's properties
	AMurphysLawPickup();

	// Called when the game starts or when spawned
	void BeginPlay() override;

	// Called when game ends
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void SetPickupVisible(bool IsVisible);

	void Respawn();
};

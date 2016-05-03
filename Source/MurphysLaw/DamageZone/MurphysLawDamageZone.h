// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "MurphysLawDamageZone.generated.h"

UCLASS()
class MURPHYSLAW_API AMurphysLawDamageZone : public AActor
{
	GENERATED_BODY()

	/** The damage inflicted to an overlapping object */
	UPROPERTY(EditDefaultsOnly, Category = "DamageZone")
	float Damage;

	/** The frequency at which damage is inflicted */
	UPROPERTY(EditDefaultsOnly, Category = "DamageZone")
	float DamageTickInterval;

	/** The collision sphere to detect near elements */
	UPROPERTY(VisibleDefaultsOnly, Category = "DamageZone")
	class USphereComponent* SphereContact;

	FTimerHandle CollisionCheckHandle;

public:	
	// Sets default values for this actor's properties
	AMurphysLawDamageZone();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when game ends
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
private:
	void DamagePlayers();
};

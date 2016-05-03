// Fill out your copyright notice in the Description page of Project Settings.

#include "MurphysLaw.h"
#include "../Character/MurphysLawCharacter.h"
#include "MurphysLawDamageZone.h"


// Sets default values
AMurphysLawDamageZone::AMurphysLawDamageZone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SphereContact = CreateDefaultSubobject<USphereComponent>(TEXT("SphereContactComponent"));
	SphereContact->SetSphereRadius(150.f, false);
	RootComponent = SphereContact;
}

// Called when the game starts or when spawned
void AMurphysLawDamageZone::BeginPlay()
{
	Super::BeginPlay();

	// Activate collision checking
	if (Role == ROLE_Authority)
	{
		GetWorldTimerManager().SetTimer(CollisionCheckHandle, this, &AMurphysLawDamageZone::DamagePlayers, DamageTickInterval, true);
	}
}

// Called when game ends
void AMurphysLawDamageZone::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	//Clear the TimerHandle_DealDamage
	GetWorldTimerManager().ClearTimer(CollisionCheckHandle);
}

void AMurphysLawDamageZone::DamagePlayers()
{
	TArray<AActor*> OverlappingActors;
	SphereContact->GetOverlappingActors(OverlappingActors);

	FHitResult HitResult;
	for(int32 i = 0; i < OverlappingActors.Num(); ++i)
	{
		AActor* OtherActor = OverlappingActors[i];
		FVector HurtDirection = OtherActor->GetActorLocation() - GetActorLocation();
		FPointDamageEvent CollisionDamageEvent(Damage, HitResult, HurtDirection, UDamageType::StaticClass());
		OtherActor->TakeDamage(Damage, CollisionDamageEvent, nullptr, this);
	}
}

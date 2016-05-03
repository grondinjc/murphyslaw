// Fill out your copyright notice in the Description page of Project Settings.

#include "MurphysLaw.h"
#include "../Interface/MurphysLawIObjectCollector.h"
#include "../Character/MurphysLawCharacter.h"
#include "MurphysLawPickup.h"
#include <MurphysLaw/Network/MurphysLawPlayerController.h>


// Sets default values
AMurphysLawPickup::AMurphysLawPickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SphereContact = CreateDefaultSubobject<USphereComponent>(TEXT("SphereContactComponent"));
	SphereContact->SetSphereRadius(50.f, false);
	RootComponent = SphereContact;
	bReplicates = true;
	bVisiblePickup = true;

	// Set the default sounds
	Sounds.CollectSound = nullptr;
}

// Called when the game starts or when spawned
void AMurphysLawPickup::BeginPlay()
{
	Super::BeginPlay();

	// Debug messages
	if (Sounds.CollectSound == nullptr) ShowWarning(FString::Printf(TEXT("[%s] - Resource 'Sounds.CollectSound' is not set"), *GetName()));

	// Bind event handlers
	SphereContact->OnComponentBeginOverlap.AddDynamic(this, &AMurphysLawPickup::OnBeginOverlap);
}

// Called when game ends
void AMurphysLawPickup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//Clear the TimerHandle_RespawnPickup
	GetWorldTimerManager().ClearTimer(TimerHandle_RespawnPickup);

	// Remove event handlers
	SphereContact->OnComponentBeginOverlap.RemoveDynamic(this, &AMurphysLawPickup::OnBeginOverlap);
}

// Event handler called when an actor overlaps the damage zone
void AMurphysLawPickup::OnBeginOverlap(AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Other Actor is the actor that triggered the event. Check that is not ourself.
	if ((OtherActor != nullptr) && (OtherActor != this))
	{
		if (OtherActor != nullptr)
		{
			AMurphysLawCharacter* Character = Cast<AMurphysLawCharacter>(OtherActor);
			// Interact with actor
			if(Character)
				ExecuteCollectorInterraction(Character);
		}
	}
}

void AMurphysLawPickup::ExecuteCollectorInterraction(AMurphysLawCharacter* Character)
{
	SetPickupVisible(false);
	
	AMurphysLawPlayerController* Controller = Cast<AMurphysLawPlayerController>(Character->GetController());

	if (Controller != nullptr && Sounds.CollectSound != nullptr)
		Controller->PickedUpItem(Sounds.CollectSound);
}

void AMurphysLawPickup::SetPickupVisible(bool IsVisible)
{
	bVisiblePickup = IsVisible;
	SetActorHiddenInGame(!IsVisible);
	SetActorEnableCollision(IsVisible);
	GetWorldTimerManager().SetTimer(TimerHandle_RespawnPickup, this, &AMurphysLawPickup::Respawn, RespawnTime, false);
}

void AMurphysLawPickup::Respawn()
{
	SetPickupVisible(true);
}


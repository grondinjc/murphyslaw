// Fill out your copyright notice in the Description page of Project Settings.

#include "MurphysLaw.h"
#include "MurphysLawShootService.h"

#include <MurphysLaw/AI/MurphysLawAIController.h>
#include <MurphysLaw/Weapon/MurphysLawBaseWeapon.h>
#include <MurphysLaw/Character/MurphysLawCharacter.h>


UMurphysLawShootService::UMurphysLawShootService()
{
	Interval = 0.4f;			// Defines timespawn between subsequent tick of the service
	RandomDeviation = 1.f;		// Added amount range to service's interval

	NodeName = "Shoot if LOS";
}

/** update next tick interval
* this function should be considered as const (don't modify state of object) if node is not instanced! */
void UMurphysLawShootService::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	AMurphysLawAIController* Controller = CastChecked<AMurphysLawAIController>(OwnerComp.GetOwner());
	
	// Retreive needed references
	AActor* Target = Controller->GetBlackboardTarget();
	AMurphysLawCharacter* Self = Controller->GetBlackboardSelfActor();

	if (Self != nullptr && Target != nullptr && Controller->LineOfSightTo(Target, FVector::ZeroVector, true))
	{
		Self->Fire();
	}
}

// Gets the description for our service
FString UMurphysLawShootService::GetStaticServiceDescription() const
{
	return TEXT("Shoot a bullet using equipped gun if there is a line of sight");
}

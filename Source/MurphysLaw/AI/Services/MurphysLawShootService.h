// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/BTService.h"
#include "MurphysLawShootService.generated.h"

/**
 * 
 */
UCLASS()
class MURPHYSLAW_API UMurphysLawShootService : public UBTService
{
	GENERATED_BODY()

public:
	UMurphysLawShootService();

protected:
	/** update next tick interval
	* this function should be considered as const (don't modify state of object) if node is not instanced! */
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// Gets the description for our service
	virtual FString GetStaticServiceDescription() const override;
};

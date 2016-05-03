// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "MurphysLawSeekTargetTask.generated.h"

/**
 * 
 */
UCLASS()
class MURPHYSLAW_API UMurphysLawSeekTargetTask : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UMurphysLawSeekTargetTask();
	
	/* Fonction d'exécution de la tâche, cette tâche devra retourner Succeeded, Failed ou InProgress */
	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** Description that will appear above node in behavior tree */
	FString GetStaticDescription() const override;

private:
	// Core function of the node; predict target position to move closer
	static FVector PredictTargetDestination(const class AActor* Target, const class AMurphysLawCharacter* Self);
};

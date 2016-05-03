// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "MurphysLawPatrolTask.generated.h"

/**
 * 
 */
UCLASS()
class MURPHYSLAW_API UMurphysLawPatrolTask : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UMurphysLawPatrolTask();

	/* Fonction d'exécution de la tâche, cette tâche devra retourner Succeeded, Failed ou InProgress */
	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** Description that will appear above node in behavior tree */
	FString GetStaticDescription() const override;
	
	
	
};

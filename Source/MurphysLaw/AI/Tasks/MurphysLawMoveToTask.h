// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "MurphysLawMoveToTask.generated.h"

/**
 * 
 */
UCLASS()
class MURPHYSLAW_API UMurphysLawMoveToTask : public UBTTask_MoveTo
{
	GENERATED_BODY()
	
public:
	UMurphysLawMoveToTask();

	/* Fonction d'exécution de la tâche, cette tâche devra retourner Succeeded, Failed ou InProgress */
	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** Description that will appear above node in behavior tree */
	FString GetStaticDescription() const override;
};

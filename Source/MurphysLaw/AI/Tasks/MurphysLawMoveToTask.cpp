// Fill out your copyright notice in the Description page of Project Settings.

#include "MurphysLaw.h"
#include "MurphysLawMoveToTask.h"



UMurphysLawMoveToTask::UMurphysLawMoveToTask()
{
	NodeName = "MoveTo";
}

/* Fonction d'exécution de la tâche, cette tâche devra retourner Succeeded, Failed ou InProgress */
EBTNodeResult::Type UMurphysLawMoveToTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	(void) Super::ExecuteTask(OwnerComp, NodeMemory);
	return EBTNodeResult::Succeeded; // Force success
}

/** Description that will appear above node in behavior tree */
FString UMurphysLawMoveToTask::GetStaticDescription() const 
{
	return TEXT("Force success to base MoveTo task");
}

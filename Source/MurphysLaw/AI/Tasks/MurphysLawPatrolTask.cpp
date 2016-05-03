// Fill out your copyright notice in the Description page of Project Settings.

#include "MurphysLaw.h"
#include "MurphysLawPatrolTask.h"

#include <MurphysLaw/AI/MurphysLawAIController.h>
#include <MurphysLaw/AI/MurphysLawAINavigationPoint.h>
#include <MurphysLaw/Character/MurphysLawCharacter.h>

UMurphysLawPatrolTask::UMurphysLawPatrolTask()
{
	NodeName = "UpdatePatrolPoint";
}

/* Fonction d'exécution de la tâche, cette tâche devra retourner Succeeded, Failed ou InProgress */
EBTNodeResult::Type UMurphysLawPatrolTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type TaskResult = EBTNodeResult::Succeeded;
	AMurphysLawAIController* Controller = CastChecked<AMurphysLawAIController>(OwnerComp.GetOwner());

	AMurphysLawAINavigationPoint* NextPatrolPoint = Controller->GetPatrolPoint();
	Controller->SetBlackboardDestination(NextPatrolPoint->GetActorLocation());
	return EBTNodeResult::Succeeded;
}

/** Description that will appear above node in behavior tree */
FString UMurphysLawPatrolTask::GetStaticDescription() const
{
	return TEXT("Set target location to patrol point");
}



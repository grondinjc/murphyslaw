// Fill out your copyright notice in the Description page of Project Settings.

#include "MurphysLaw.h"
#include "MurphysLawSeekTargetTask.h"

#include <MurphysLaw/AI/MurphysLawAIController.h>
#include <MurphysLaw/Character/MurphysLawCharacter.h>


UMurphysLawSeekTargetTask::UMurphysLawSeekTargetTask()
{
	NodeName = "UpdateNextTargetPoint";
}

/* Fonction d'exécution de la tâche, cette tâche devra retourner Succeeded, Failed ou InProgress */
EBTNodeResult::Type UMurphysLawSeekTargetTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type TaskResult = EBTNodeResult::Succeeded;
	AMurphysLawAIController* Controller = CastChecked<AMurphysLawAIController>(OwnerComp.GetOwner());

	// Retreive needed references
	AActor* Target = Controller->GetBlackboardTarget();
	AMurphysLawCharacter* Self = Controller->GetBlackboardSelfActor();
	
	if(Target != nullptr && Self != nullptr)
	{
		const FVector PredictedDestination = PredictTargetDestination(Target, Self);
		Controller->SetBlackboardDestination(PredictedDestination);
	}
	else
	{
		TaskResult = EBTNodeResult::Failed;
	}

	return TaskResult;
}

// static
// Will probably end up in EQS query
FVector UMurphysLawSeekTargetTask::PredictTargetDestination(const AActor* Target, const AMurphysLawCharacter* Self)
{
	// Compute predicted destination of target
	return Target->GetActorLocation() + Target->GetVelocity();;
}

/** Description that will appear above node in behavior tree */
FString UMurphysLawSeekTargetTask::GetStaticDescription() const
{
	return TEXT("Compute near location to target");
}

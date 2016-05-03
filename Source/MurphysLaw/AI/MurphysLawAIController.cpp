// Fill out your copyright notice in the Description page of Project Settings.

#include "MurphysLaw.h"
#include "MurphysLawAIController.h"
#include <MurphysLaw/Network/MurphysLawGameMode.h>
#include <MurphysLaw/Network/MurphysLawPlayerState.h>
#include <MurphysLaw/Character/MurphysLawCharacter.h>
#include <MurphysLaw/Utils/MurphysLawUtils.h>
#include <MurphysLaw/Character/MurphysLawCharacter.h>
#include <MurphysLaw/AI/MurphysLawAINavigationPoint.h>

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/Actor.h"
#include "Navigation/CrowdFollowingComponent.h"

// Define constants.
// NAME MUST ABSOLUTELY MATCH THE BLACKBOARD ASSET DEFINED IN BLUEPRINT !!
const FName AMurphysLawAIController::KEYNAME_SELFACTOR("SelfActor");
const FName AMurphysLawAIController::KEYNAME_CANMOVE("CanMove");
const FName AMurphysLawAIController::KEYNAME_DESTINATION("Destination");
const FName AMurphysLawAIController::KEYNAME_TARGET("Target");


AMurphysLawAIController::AMurphysLawAIController()
	// Enable navigation within crowds
	//: Super(FObjectInitializer::Get().SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
	bWantsPlayerState = true;
	// bAllowStrafe = true; // ?? Run and shoot
	
	// The action logic of the AI (assigned from blueprint)
	BehaviorTreeAsset = nullptr;

	// No pawn is yet controlled
	bPossessPawn = false;

	// Create and configure sight sense
	UAISenseConfig_Sight* SightSenseConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("Sight sense");
	SightSenseConfig->PeripheralVisionAngleDegrees = 90.f;
	SightSenseConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightSenseConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightSenseConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightSenseConfig->SightRadius = 100.f * 20.f;
	SightSenseConfig->LoseSightRadius = SightSenseConfig->SightRadius * 1.2f;
	SightSenseConfig->AutoSuccessRangeFromLastSeenLocation = SightSenseConfig->SightRadius * 1.1f;

	UAISenseConfig_Hearing* HearingSenseConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>("Hearing sense");
	HearingSenseConfig->HearingRange = 100.f * 50.f;
	HearingSenseConfig->LoSHearingRange = 100.f * 150.f;
	HearingSenseConfig->bUseLoSHearing = true;
	HearingSenseConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingSenseConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingSenseConfig->DetectionByAffiliation.bDetectFriendlies = true;

	// Create perceptions system
	UAIPerceptionComponent* Perceptions = CreateDefaultSubobject<UAIPerceptionComponent>("PerceptionComp");
	Perceptions->ConfigureSense(*SightSenseConfig);
	Perceptions->ConfigureSense(*HearingSenseConfig);
	Perceptions->SetDominantSense(SightSenseConfig->GetSenseImplementation());
	Perceptions->Deactivate();
	SetPerceptionComponent(*Perceptions);
}

void AMurphysLawAIController::BeginPlay()
{
	Super::BeginPlay();

	// Detect navigation points for patrol state
	NavigationPoints = MurphysLawUtils::GetAllSceneReferences<AMurphysLawAINavigationPoint>(this);

	// Bind events callbacks
	if(GetPerceptionComponent())
	{
		GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &AMurphysLawAIController::OnTargetPerceptionUpdated);
	}

	// Make sure the ressource is assigned
	if (BehaviorTreeAsset == nullptr) ShowError("No behavior tree assigned to AI controller");
	else if (BehaviorTreeAsset->BlackboardAsset == nullptr) ShowError("No blackboard assigned to AI controller's behavior tree");
}

void AMurphysLawAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Remove bindings
	if (GetPerceptionComponent())
	{
		GetPerceptionComponent()->OnTargetPerceptionUpdated.RemoveDynamic(this, &AMurphysLawAIController::OnTargetPerceptionUpdated);
	}
}

void AMurphysLawAIController::Possess(APawn* InPawn)
{
	Super::Possess(InPawn);

	// Reset the blackboard and start ai action logic
	if (BehaviorTreeAsset != nullptr && InPawn != nullptr)
	{
		bPossessPawn = true;

		// Based on AAIController::RunBehaviorTree implementation, the
		// AAIController::Blackboard variable should be initialized on success
		const bool StartedSuccessfully = RunBehaviorTree(BehaviorTreeAsset);
		if(StartedSuccessfully && Blackboard != nullptr)
		{
			// Set initial default values
			InitializeBlackboardKeys(InPawn);
		}
		else
		{
			ShowError("Blackboard component should have been created by parent class");
		}
	}
}

// Code is executed on blueprint subclass re-compilation
// Therefore, not all pointers are granted to be valid
void AMurphysLawAIController::UnPossess()
{
	Super::UnPossess();
	bPossessPawn = false;

	static const FString StopReason("No pawn to control");
	if (BrainComponent && BrainComponent->IsRunning()) BrainComponent->StopLogic(StopReason);
}

void AMurphysLawAIController::InitializeBlackboardKeys(APawn* InPawn)
{
	AMurphysLawCharacter* Character = CastChecked<AMurphysLawCharacter>(InPawn);

	// Save controller pawn
	SetBlackboardSelfActor(Character);
	SetBlackboardCanMove(false);
	SetBlackboardTarget(nullptr);
	SetBlackboardDestination(FVector::ZeroVector);
}

void AMurphysLawAIController::OnTargetPerceptionUpdated(AActor* UpdatedActor, FAIStimulus Stimulus)
{
	// Don't handle events if no pawn is controlled
	if (bPossessPawn && UpdatedActor != GetPawn())
	{
		if(!MurphysLawUtils::IsInSameTeam(GetPawn(), UpdatedActor))
		{
			AMurphysLawCharacter* UpdatedCharacter = CastChecked<AMurphysLawCharacter>(UpdatedActor);
			SetFocus(UpdatedCharacter->GetFocalPoint(), EAIFocusPriority::Gameplay);
			SetBlackboardTarget(UpdatedActor);
		}
	}
}

// Event when controlled pawn has died
void AMurphysLawAIController::OnKilled(const float TimeToRespawn)
{
	// Disable movement task branch from BT and current Move task
	SetBlackboardCanMove(false);
	StopMovement();

	SetBlackboardTarget(nullptr);

	// Stop looking at target
	// Note :	Calling ClearFocus changes the current rotation.
	//			Saving the rotation avoid instant turn upon death
	const FRotator CurrentRotation = GetPawn()->GetActorRotation();
	ClearFocus(EAIFocusPriority::Gameplay);
	ClearFocus(EAIFocusPriority::Default);
	ClearFocus(EAIFocusPriority::LastFocusPriority);
	ClearFocus(EAIFocusPriority::Move);
	GetPawn()->SetActorRotation(CurrentRotation);

	// Respawn logic
	MyCharacter = Cast<AMurphysLawCharacter>(this->GetPawn());
	UnPossess();
	GetWorldTimerManager().SetTimer(TimerHandle_Respawn, this, &AMurphysLawAIController::Respawn, TimeToRespawn);
}


void AMurphysLawAIController::Respawn()
{
	AMurphysLawGameMode* GameMode = Cast<AMurphysLawGameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		AMurphysLawPlayerState* MurphysLawPlayerState = Cast<AMurphysLawPlayerState>(PlayerState);
		MurphysLawUtils::RespawnCharacter(GameMode, MurphysLawPlayerState, MyCharacter);
		Possess(MyCharacter);
		SetBlackboardCanMove(true);
	}
}


AMurphysLawCharacter* AMurphysLawAIController::GetBlackboardSelfActor() const { return Cast<AMurphysLawCharacter>(Blackboard->GetValueAsObject(KEYNAME_SELFACTOR)); }
void AMurphysLawAIController::SetBlackboardSelfActor(AMurphysLawCharacter* Self) { Blackboard->SetValueAsObject(KEYNAME_SELFACTOR, Self); }

bool AMurphysLawAIController::GetBlackboardCanMove() const { return Blackboard->GetValueAsBool(KEYNAME_CANMOVE); }
void AMurphysLawAIController::SetBlackboardCanMove(const bool CanMove) { Blackboard->SetValueAsBool(KEYNAME_CANMOVE, CanMove); }

FVector AMurphysLawAIController::GetBlackboardDestination() const { return Blackboard->GetValueAsVector(KEYNAME_DESTINATION); }
void AMurphysLawAIController::SetBlackboardDestination(const FVector Destination) { Blackboard->SetValueAsVector(KEYNAME_DESTINATION, Destination); }

AActor* AMurphysLawAIController::GetBlackboardTarget() const { return Cast<AActor>(Blackboard->GetValueAsObject(KEYNAME_TARGET)); }
void AMurphysLawAIController::SetBlackboardTarget(AActor* Target) { Blackboard->SetValueAsObject(KEYNAME_TARGET, Target); }

AMurphysLawAINavigationPoint* AMurphysLawAIController::GetPatrolPoint()
{
	checkf(NavigationPoints.Num() > 0, TEXT("No navigation points detected"));
	return NavigationPoints[FMath::RandRange(0, NavigationPoints.Num() - 1)];
}

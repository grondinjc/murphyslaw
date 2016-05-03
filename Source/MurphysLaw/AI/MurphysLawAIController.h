// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Perception/AIPerceptionTypes.h"
#include "DetourCrowdAIController.h"
#include "AIController.h"
#include <MurphysLaw/Interface/MurphysLawIController.h>
#include "MurphysLawAIController.generated.h"


/**
 * 
 */
UCLASS()
class MURPHYSLAW_API AMurphysLawAIController : public AAIController, public IMurphysLawIController
{
	GENERATED_BODY()

	// NAME MUST ABSOLUTELY MATCH THE BLACKBOARD ASSET DEFINED IN BLUEPRINT !!!
	static const FName KEYNAME_SELFACTOR;
	static const FName KEYNAME_CANMOVE;
	static const FName KEYNAME_DESTINATION;
	static const FName KEYNAME_TARGET;

	// The logic of Ai actions (set in subclass)
	UPROPERTY(EditDefaultsOnly, Category = "AI Blackboard")
	class UBehaviorTree* BehaviorTreeAsset;

	// Patrol points when no target is set
	TArray<class AMurphysLawAINavigationPoint*> NavigationPoints;

	/* Flag indication that the controller possesses a pawn.
	Helps avoiding handling events when no pawn is controlled.*/
	bool bPossessPawn;

	/** Keeps an instance on the character this PlayerController controls */
	class AMurphysLawCharacter* MyCharacter;

	void InitializeBlackboardKeys(class APawn* InPawn);

public:
	AMurphysLawAIController();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Possess(APawn* InPawn) override;
	virtual void UnPossess() override;

	// Event when controlled pawn has died
	void OnKilled(const float TimeToRespawn) override;
	void Respawn() override;

	/** Notification of perception changes in given actors' perception */
	UFUNCTION()
	void OnTargetPerceptionUpdated(class AActor* UpdatedActor, FAIStimulus Stimulus);


	UFUNCTION(BlueprintCallable, Category = "AI Blackboard")
	class AMurphysLawCharacter* GetBlackboardSelfActor() const;
	UFUNCTION(BlueprintCallable, Category = "AI Blackboard")
	void SetBlackboardSelfActor(class AMurphysLawCharacter* Self);

	UFUNCTION(BlueprintCallable, Category = "AI Blackboard")
	bool GetBlackboardCanMove() const;
	UFUNCTION(BlueprintCallable, Category = "AI Blackboard")
	void SetBlackboardCanMove(const bool CanMove);

	UFUNCTION(BlueprintCallable, Category = "AI Blackboard")
	FVector GetBlackboardDestination() const;
	UFUNCTION(BlueprintCallable, Category = "AI Blackboard")
	void SetBlackboardDestination(const FVector DestinationLocation);

	UFUNCTION(BlueprintCallable, Category = "AI Blackboard")
	class AActor* GetBlackboardTarget() const;
	UFUNCTION(BlueprintCallable, Category = "AI Blackboard")
	void SetBlackboardTarget(class AActor* Self);

	class AMurphysLawAINavigationPoint* GetPatrolPoint();

private:
	/** Handle for efficient management of the Respawn timer */
	FTimerHandle TimerHandle_Respawn;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameState.h"
#include "MurphysLawGameState.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class MurphysLawMatchState : uint8
{
	EInLobby		UMETA(DisplayName = "InLobby"),
	EWarmUp			UMETA(DisplayName = "WarmUp"),
	EPlaying		UMETA(DisplayName = "Playing"),
	EScoreBoard		UMETA(DisplayName = "Scoreboard")
};

UCLASS()
class MURPHYSLAW_API AMurphysLawGameState : public AGameState
{
	GENERATED_BODY()

	static const int32 SUICIDE_POINT = -5;
	static const int32 KILL_POINT = 10;
	static const int32 TEAMMATEKILL_POINT = -5;

public:
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "GameState")
	int32 RemainingTime;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "GameState")
	int32 ScoreTeamA;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "GameState")
	int32 ScoreTeamB;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "GameState")
	int32 WinningTeam;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadWrite, Category = "GameState")
	MurphysLawMatchState MurphysLawMatchState = MurphysLawMatchState::EInLobby;

	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
	void ResetStats();

	UFUNCTION(BlueprintCallable, Category = "GameState")
	FString GetFormattedRemainingTime();

	void PlayerCommitedSuicide(bool isTeamA);
	void PlayerWasKilled(bool isTeamA);
	void PlayerKilledTeammate(bool isTeamA);
};

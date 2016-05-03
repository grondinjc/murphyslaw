// Fill out your copyright notice in the Description page of Project Settings.

#include "MurphysLaw.h"
#include "MurphysLawGameState.h"

void AMurphysLawGameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMurphysLawGameState, RemainingTime);
	DOREPLIFETIME(AMurphysLawGameState, MurphysLawMatchState);
	DOREPLIFETIME(AMurphysLawGameState, ScoreTeamA);
	DOREPLIFETIME(AMurphysLawGameState, ScoreTeamB);
	DOREPLIFETIME(AMurphysLawGameState, WinningTeam);
}

void AMurphysLawGameState::ResetStats()
{
	RemainingTime = 0.f;
	ScoreTeamA = 0;
	ScoreTeamB = 0;
}

FString AMurphysLawGameState::GetFormattedRemainingTime()
{
	uint32 Minutes = RemainingTime / 60;
	uint32 Seconds = int(RemainingTime % 60);
	
	FString SSeconds = Seconds < 10 ? FString::Printf(TEXT("0%d"), Seconds) : FString::FromInt(Seconds);

	return FString::Printf(TEXT("%d:"), Minutes) + SSeconds;
}

void AMurphysLawGameState::PlayerCommitedSuicide(bool isTeamA)
{
	(isTeamA ? ScoreTeamA : ScoreTeamB) += SUICIDE_POINT;
}

void AMurphysLawGameState::PlayerWasKilled(bool isTeamA)
{
	(isTeamA ? ScoreTeamA : ScoreTeamB) += KILL_POINT;
}

void AMurphysLawGameState::PlayerKilledTeammate(bool isTeamA)
{
	(isTeamA ? ScoreTeamA : ScoreTeamB) += TEAMMATEKILL_POINT;
}
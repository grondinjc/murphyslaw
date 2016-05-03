// Fill out your copyright notice in the Description page of Project Settings.

#include "MurphysLaw.h"
#include "MurphysLawScoreboardWidget.h"
#include "../Network/MurphysLawPlayerState.h"
#include "../Network/MurphysLawGameMode.h"
#include "../Network/MurphysLawPlayerController.h"
#include "../Network/MurphysLawGameState.h"

void UMurphysLawScoreboardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	IsScoreboardVisible = false;
}

// Sets the character who's information will be shown on the widget
void UMurphysLawScoreboardWidget::SetPlayerController(AMurphysLawPlayerController* PlayerController)
{
	if (PlayerController)
	{
		MyPlayerController = PlayerController;
	}

	checkf(MyPlayerController != nullptr, TEXT("Scoreboard - MyPlayerController is null"));
}

// Generates the data of Team A for the Scoreboard
FText UMurphysLawScoreboardWidget::GetTeamAData() const
{
	AMurphysLawGameState* GameState = Cast<AMurphysLawGameState>(GetWorld()->GetGameState());
	if (GameState)
		return FText::FromString(FString::Printf(TEXT("---------- TEAM A Score : %d ----------\n"), GameState->ScoreTeamA) +
			GetScoreboardData(AMurphysLawGameMode::TEAM_A));
	else
		return FText();
}

// Generates the data of Team A for the Scoreboard
FText UMurphysLawScoreboardWidget::GetTeamBData() const
{
	AMurphysLawGameState* GameState = Cast<AMurphysLawGameState>(GetWorld()->GetGameState());
	if (GameState)
		return FText::FromString(FString::Printf(TEXT("---------- TEAM B Score : %d ----------\n"), GameState->ScoreTeamB) +
			GetScoreboardData(AMurphysLawGameMode::TEAM_B));
	else
		return FText();
}

// Generates the text for the Scoreboard
FString UMurphysLawScoreboardWidget::GetScoreboardData(int32 NoTeam) const
{
	AGameState* GameState = GetWorld()->GetGameState();
	FString Result = "";

	if (GameState != nullptr)
	{
		// Loops through all the players and calculates their stats
		for (APlayerState* It : GameState->PlayerArray)
		{
			AMurphysLawPlayerState* PlayerState = Cast<AMurphysLawPlayerState>(It);
			if (PlayerState != nullptr && PlayerState->GetTeam() == NoTeam && PlayerState->IsActive())
			{
				Result += (MyPlayerController && MyPlayerController->PlayerState == It ? TEXT("* ") : TEXT("  "));
				Result += PlayerState->GetHumanReadableName() + TEXT("    NbKills: ") + FString::FromInt(PlayerState->GetNbKills());
				Result += TEXT("    NbDeaths: ") + FString::FromInt(PlayerState->GetNbDeaths());
				Result += TEXT("\n");
			}
			else if (PlayerState == nullptr)
			{
				ShowError(TEXT("Unable to cast player state to AMurphysLawPlayerState"));
			}
		}
	}
	else
	{
		ShowError(TEXT("Unable to show the list of player in the game"));
	}

	return Result;
}

// Reports the Scoreboard visibility
ESlateVisibility UMurphysLawScoreboardWidget::GetWinningTeamVisibility() const
{
	AMurphysLawGameState* GameState = Cast<AMurphysLawGameState>(GetWorld()->GetGameState());
	if(GameState)
		return GameState->MurphysLawMatchState == MurphysLawMatchState::EScoreBoard ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
	return ESlateVisibility::Hidden;
}

FString UMurphysLawScoreboardWidget::GetWinningTeam() const
{
	AMurphysLawGameState* GameState = Cast<AMurphysLawGameState>(GetWorld()->GetGameState());
	if (GameState)
	{
		FString WinningTeam = (GameState->WinningTeam == AMurphysLawGameMode::DRAW) ? "Draw" : (GameState->WinningTeam == AMurphysLawGameMode::TEAM_A ? "A" : "B");
		return "Winning team: " + WinningTeam;
	}
	return "";
}

// Reports the Scoreboard visibility
ESlateVisibility UMurphysLawScoreboardWidget::GetScoreboardVisibility() const
{
	return IsScoreboardVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
}

// Shows the scoreboard
void UMurphysLawScoreboardWidget::Show()
{
	IsScoreboardVisible = true;
}

// Hides the scoreboard
void UMurphysLawScoreboardWidget::Hide()
{
	IsScoreboardVisible = false;
}
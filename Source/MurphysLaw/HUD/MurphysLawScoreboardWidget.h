// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "MurphysLawScoreboardWidget.generated.h"

/**
 * 
 */
UCLASS()
class MURPHYSLAW_API UMurphysLawScoreboardWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/** Called on construction */
	void NativeConstruct() override;

	/** Sets the character who's information will be shown on the widget */
	void SetPlayerController(class AMurphysLawPlayerController* PlayerController);

	/** Shows the scoreboard */
	void Show();

	/** Hides the scoreboard */
	void Hide();

protected:
	/** Keeps a reference to the character owning the HUD */
	UPROPERTY(VisibleAnywhere)
	class AMurphysLawPlayerController* MyPlayerController;

	/** Reports the scoreboard visibility */
	UFUNCTION(BlueprintPure, Category = "Scoreboard")
	ESlateVisibility GetScoreboardVisibility() const;

	UFUNCTION(BlueprintPure, Category = "Scoreboard")
		ESlateVisibility GetWinningTeamVisibility() const;

	/** Generates the data of Team A for the Scoreboard */
	UFUNCTION(BlueprintPure, Category = "Scoreboard")
	FText GetTeamAData() const;

	/** Generates the data of Team A for the Scoreboard */
	UFUNCTION(BlueprintPure, Category = "Scoreboard")
	FText GetTeamBData() const;

	/** Generates the data of Team A for the Scoreboard */
	UFUNCTION(BlueprintPure, Category = "Scoreboard")
	FString GetWinningTeam() const;

private:
	/** Generates the data of a team for the Scoreboard */
	FString GetScoreboardData(int32 NoTeam) const;

	/** Tells whether the scoreboard is shown or not */
	bool IsScoreboardVisible;
	
};

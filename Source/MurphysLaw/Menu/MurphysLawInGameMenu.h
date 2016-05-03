// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "MurphysLawInGameMenu.generated.h"

/**
 * 
 */
UCLASS()
class MURPHYSLAW_API UMurphysLawInGameMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void OnResumeGameClick();
	
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void OnQuitSessionClick();

	/** Called by the character who spawned the Menu to keep a reference to it */
	void SetPlayerController(class AMurphysLawPlayerController* PlayerController);

protected:
	/** Called when the player presses a key and this widget has focus */
	FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	/** The reference to the character who uses the Menu */
	class AMurphysLawPlayerController* MyPlayerController;
};

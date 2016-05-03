// Fill out your copyright notice in the Description page of Project Settings.

#include "MurphysLaw.h"
#include "MurphysLawInGameMenu.h"
#include "../Network/MurphysLawPlayerController.h"
#include "Character/MurphysLawCharacter.h"
#include "../MurphysLawGameInstance.h"

// Called by the character who spawned the Menu to keep a reference to it
void UMurphysLawInGameMenu::SetPlayerController(class AMurphysLawPlayerController* PlayerController)
{
	MyPlayerController = PlayerController;

	checkf(MyPlayerController != nullptr, TEXT("InGameMenu - MyPlayerController is null"));
}

void UMurphysLawInGameMenu::OnResumeGameClick()
{
	MyPlayerController->ToggleInGameMenu();
}

void UMurphysLawInGameMenu::OnQuitSessionClick()
{
	if (MyPlayerController->GetWorld() != nullptr && MyPlayerController->GetWorld()->GetGameInstance() != nullptr)
	{
		UMurphysLawGameInstance* GameInstance = Cast<UMurphysLawGameInstance>(MyPlayerController->GetWorld()->GetGameInstance());
		if (GameInstance)
			GameInstance->DestroySessionAndLeaveGame();
	}
	else
		ShowError("OnQuitSessionClick nullptr");
}

/** Called when the player presses a key and this widget has focus */
FReply UMurphysLawInGameMenu::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		OnResumeGameClick();
		return FReply::Handled();
	}

	return FReply::Unhandled();
}
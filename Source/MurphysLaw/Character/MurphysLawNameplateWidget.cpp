// Fill out your copyright notice in the Description page of Project Settings.

#include "MurphysLaw.h"
#include "MurphysLawNameplateWidget.h"
#include "MurphysLawCharacter.h"
#include "../Utils/MurphysLawUtils.h"

// Formats the name of the character to be displayed
FText UMurphysLawNameplateWidget::GetCharacterNameText() const
{
	FString Name = "MyCharacter == nullptr";
	if (MyCharacter && MyCharacter->PlayerState)
	{
		Name = MyCharacter->PlayerState->GetHumanReadableName();
	}

	return FText::FromString(Name);
}

// Sets the character who's information will be shown on the widget
void UMurphysLawNameplateWidget::SetCharacter(AMurphysLawCharacter* Character)
{
	MyCharacter = Character;

	checkf(MyCharacter != nullptr, TEXT("Nameplate Character is null"));
}

ESlateVisibility UMurphysLawNameplateWidget::GetVisibility() const
{
	ACharacter* OtherCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

	return MyCharacter != nullptr && OtherCharacter != nullptr && MurphysLawUtils::IsInSameTeam(OtherCharacter->PlayerState, MyCharacter->PlayerState)
		? ESlateVisibility::Visible 
		: ESlateVisibility::Collapsed;
}
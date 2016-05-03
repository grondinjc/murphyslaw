// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "MurphysLawNameplateWidget.generated.h"

/**
 * 
 */
UCLASS()
class MURPHYSLAW_API UMurphysLawNameplateWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	/** Keeps a reference to the character */
	UPROPERTY(VisibleAnywhere)
	class AMurphysLawCharacter* MyCharacter;
	
public:
	/** Formats the name of the character to be displayed */
	UFUNCTION(BlueprintPure, Category = "Name")
	FText GetCharacterNameText() const;
	
	/** Sets the character who's information will be shown on the widget */
	void SetCharacter(class AMurphysLawCharacter* Character);

	UFUNCTION(BlueprintPure, Category = "Name")
	ESlateVisibility GetVisibility() const;
	
};
// Fill out your copyright notice in the Description page of Project Settings.

#include "MurphysLaw.h"
#include "MurphysLawHUDWidget.h"
#include "../Character/MurphysLawCharacter.h"
#include "../Weapon/MurphysLawBaseWeapon.h"
#include "../Network/MurphysLawPlayerController.h"
#include "../Network/MurphysLawGameState.h"
#include "Kismet/KismetMathLibrary.h"

class AMurphysLawGameState;
DEFINE_LOG_CATEGORY(ML_HUDWidget);

UMurphysLawHUDWidget::UMurphysLawHUDWidget(const FObjectInitializer& ObjectInitializer)
	: UUserWidget(ObjectInitializer)
{
	HitMarkerOpacity = 0.f;
	DamageIndicatorOpacity = 0.f;
}

// Sets the character who's information will be shown on the widget
void UMurphysLawHUDWidget::SetPlayerController(AMurphysLawPlayerController* PlayerController)
{
	if (PlayerController)
	{		
		MyPlayerController = PlayerController;
		MyCharacter = PlayerController->GetMyCharacter();
	}

	checkf(MyPlayerController != nullptr, TEXT("HUDWidget - MyPlayerController is null"));
	checkf(MyCharacter != nullptr, TEXT("HUDWidget - MyCharacter is null"));
}

// Adjust the character's offset on the minimap
FVector UMurphysLawHUDWidget::GetMiniMapActorLocation() const
{
	return MyCharacter->GetActorLocation();
}

// Calculates the character's angle on the minimap for the rotation
float UMurphysLawHUDWidget::GetMiniMapAngle() const
{	
	return MyCharacter->GetBearing() * -1.0f;
}

// Calculates the health percent of the character
float UMurphysLawHUDWidget::GetHealthPercent() const
{
	return MyCharacter->GetCurrentHealthLevel() / MyCharacter->GetMaxHealthLevel();
}

// Formats the number of ammo of the character to be displayed
FText UMurphysLawHUDWidget::GetAmountOfAmmoText() const
{
	int32 AmountOfAmmoInWeapon = 0;
	int32 AmountOfAmmoInInventory = 0;

	if (MyCharacter->HasWeaponEquipped())
	{
		AmountOfAmmoInWeapon = MyCharacter->GetEquippedWeapon()->GetNumberOfAmmoLeftInMagazine();
		AmountOfAmmoInInventory = MyCharacter->GetEquippedWeapon()->GetNumberOfAmmoLeftInInventory();
	}

	return FText::Format(FText::FromString("{0} | {1}"), FText::AsNumber(AmountOfAmmoInWeapon), FText::AsNumber(AmountOfAmmoInInventory));
}

// Reports the Game Timer visibility
ESlateVisibility UMurphysLawHUDWidget::GetGameTimerVisibility() const
{
	AMurphysLawGameState* GameState = Cast<AMurphysLawGameState>(GetWorld()->GetGameState());
	return GameState != nullptr && GameState->MurphysLawMatchState == MurphysLawMatchState::EPlaying ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
}

// Reports the Warm Up Timer visibility
ESlateVisibility UMurphysLawHUDWidget::GetWarmUpTimerVisibility() const
{
	AMurphysLawGameState* GameState = Cast<AMurphysLawGameState>(GetWorld()->GetGameState());
	return GameState != nullptr && GameState->MurphysLawMatchState == MurphysLawMatchState::EWarmUp ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
}

// Gets the OnScreenMessages string
FText UMurphysLawHUDWidget::GetOnScreenMessages() const
{
	return FText::FromString(OnScreenMessages);
}

// Adds a message to be shown on the screen
void UMurphysLawHUDWidget::AddOnScreenMessage(FString InMessage)
{
	// Adds the message at the bottom of the stack
	OnScreenMessages = OnScreenMessages.Append(InMessage + "\n");

	// And starts a timer for the message to be removed
	FTimerHandle TimerHandler;
	GetWorld()->GetTimerManager().SetTimer(TimerHandler, this, &UMurphysLawHUDWidget::RemoveFirstOnScreenMessage, REMOVE_MESSAGE_DELAY);
}

// Removes the first message of the stack
void UMurphysLawHUDWidget::RemoveFirstOnScreenMessage()
{
	FString LeftPart, RightPart;

	// Splits the string on the first occurence of a line break
	if (OnScreenMessages.Split(TEXT("\n"), &LeftPart, &RightPart))
	{		
		// And stores the rest of the messages back in the OnScreenMessages
		OnScreenMessages = RightPart;
	}
	else
	{
		OnScreenMessages = TEXT("");
	}
}

// Calculates the stamina percentage of the character
float UMurphysLawHUDWidget::GetStaminaPercent() const
{
	return MyCharacter->GetCurrentStaminaLevel() / MyCharacter->GetMaxStaminaLevel();
}

// Reports the equipped weapon name
FText UMurphysLawHUDWidget::GetWeaponName() const
{
	FString Name = TEXT("-- No Name --");

	if (MyCharacter->HasWeaponEquipped())
	{
		Name = MyCharacter->GetEquippedWeapon()->GetWeaponName();
	}

	return FText::FromString(Name);
}

// Changes HitMarker opacity to the max
void UMurphysLawHUDWidget::ShowHitMarker()
{
	bool MustStartTimer = HitMarkerOpacity == 0.f;	
	HitMarkerOpacity = 1.f;

	if (MustStartTimer)
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UMurphysLawHUDWidget::LowerHitMarkerOpacity, MARKERS_OPACITY_REDUCE_DELAY);
	}
}

// Reports the hitmarker color and opacity
FLinearColor UMurphysLawHUDWidget::GetHitMarkerColorAndOpacity() const
{
	return FLinearColor(1.f, 1.f, 1.f, HitMarkerOpacity);
}

// Lowers the crosshair opacity
void UMurphysLawHUDWidget::LowerHitMarkerOpacity()
{
	HitMarkerOpacity = UKismetMathLibrary::FMax(HitMarkerOpacity - HITMARKER_OPACITY_REDUCE_RATE, 0.f);
	if (HitMarkerOpacity > 0.f)
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UMurphysLawHUDWidget::LowerHitMarkerOpacity, MARKERS_OPACITY_REDUCE_DELAY);
	}
}

// Implementable event in Blueprint that will change the orientation of the DamageIndicator
void UMurphysLawHUDWidget::ShowDamage(float Angle)
{
	bool MustStartTimer = DamageIndicatorOpacity == 0.f;
	DamageIndicatorOpacity = 1.f;

	if (MustStartTimer)
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UMurphysLawHUDWidget::LowerDamageIndicatorOpacity, MARKERS_OPACITY_REDUCE_DELAY);
	}

	// Calls the blueprint event to actually change the rotation of the damage indicator
	this->OnShowDamage(Angle);
}

// Reports the DamageIndicator color and opacity
FLinearColor UMurphysLawHUDWidget::GetDamageIndicatorColorAndOpacity() const
{
	return FLinearColor(1.f, 0.f, 0.f, DamageIndicatorOpacity);
}

/** Lowers the DamageIndicator opacity */
void UMurphysLawHUDWidget::LowerDamageIndicatorOpacity()
{
	DamageIndicatorOpacity = UKismetMathLibrary::FMax(DamageIndicatorOpacity - DAMAGE_INDICATOR_OPACITY_REDUCE_RATE, 0.f);
	if (DamageIndicatorOpacity > 0.f)
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UMurphysLawHUDWidget::LowerDamageIndicatorOpacity, MARKERS_OPACITY_REDUCE_DELAY);
	}
}
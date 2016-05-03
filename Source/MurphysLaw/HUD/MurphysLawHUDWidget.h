// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "MurphysLawHUDWidget.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(ML_HUDWidget, Log, All);

/**
 * 
 */
UCLASS()
class MURPHYSLAW_API UMurphysLawHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMurphysLawHUDWidget(const FObjectInitializer& ObjectInitializer);

	/** Accessor function for the character's location minus the starting offset */
	UFUNCTION(BlueprintPure, Category = "MiniMap")
	FVector GetMiniMapActorLocation() const;

	/** Accessor function for the MiniMap angle */
	UFUNCTION(BlueprintPure, Category = "MiniMap")
	float GetMiniMapAngle() const;

	/** Sets the character who's information will be shown on the widget */
	void SetPlayerController(class AMurphysLawPlayerController* PlayerController);

	/** Calculates the health percent of the character */
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const;

	/** Formats the number of ammo of the character to be displayed */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	FText GetAmountOfAmmoText() const;

	/** Reports the Game Timer visibility */
	UFUNCTION(BlueprintPure, Category = "Timer")
	ESlateVisibility GetGameTimerVisibility() const;
	
	/** Reports the Warm Up Timer visibility */
	UFUNCTION(BlueprintPure, Category = "Timer")
	ESlateVisibility GetWarmUpTimerVisibility() const;

	/** Adds a message to be shown on the screen */
	void AddOnScreenMessage(FString InMessage);

	/** Changes HitMarker opacity to the max */
	void ShowHitMarker();

	/** Changes DamageIndicator opacity to the max */
	void ShowDamage(float Angle);

protected:
	/** Keeps a reference to the character owning the HUD */
	UPROPERTY(VisibleAnywhere)
	class AMurphysLawPlayerController* MyPlayerController;

	/** The size of the minimap image */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	float MapSize;

	/** The zoom applied to the minimap image */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	float MapZoom;

	/** Gets the OnScreenMessages string */
	UFUNCTION(BlueprintPure, Category = "OnScreenMessage")
	FText GetOnScreenMessages() const;

	/** Calculates the stamina percentage of the character */
	UFUNCTION(BlueprintPure, Category = "Stamina")
	float GetStaminaPercent() const;

	/** Reports the equipped weapon name */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	FText GetWeaponName() const;

	/** Reports the hitmarker color and opacity */
	UFUNCTION(BlueprintPure, Category = "HitMarker")
	FLinearColor GetHitMarkerColorAndOpacity() const;

	/** Reports the DamageIndicator color and opacity */
	UFUNCTION(BlueprintPure, Category = "Damage")
	FLinearColor GetDamageIndicatorColorAndOpacity() const;

	/** Implementable event in Blueprint that will change the orientation of the DamageIndicator */
	UFUNCTION(BlueprintImplementableEvent, Category = "Damage")
	void OnShowDamage(float Angle);

private:
	/** The delay before an OnScreenMessage is removed */
	const float REMOVE_MESSAGE_DELAY = 3.f;

	/** The delay before the HitMarker opacity is lowered */
	const float MARKERS_OPACITY_REDUCE_DELAY = 0.07f;

	/** The rate at which the opacity of the HitMarker fades away */
	const float HITMARKER_OPACITY_REDUCE_RATE = 0.125f;

	/** The rate at which the opacity of the Damage Indication fades away */
	const float DAMAGE_INDICATOR_OPACITY_REDUCE_RATE = 0.075f;

	/** Represents the HitMarker opacity */
	float HitMarkerOpacity;

	/** Represents the DamageIndicator opacity */
	float DamageIndicatorOpacity;

	/** Keeps the character instance from the PlayerController */
	class AMurphysLawCharacter* MyCharacter;

	/** Keeps all the OnScreen messages to be shown */
	FString OnScreenMessages;

	/** Removes the first message of the stack */
	void RemoveFirstOnScreenMessage();

	/** Lowers the HitMarker opacity */
	void LowerHitMarkerOpacity();

	/** Lowers the DamageIndicator opacity */
	void LowerDamageIndicatorOpacity();
};

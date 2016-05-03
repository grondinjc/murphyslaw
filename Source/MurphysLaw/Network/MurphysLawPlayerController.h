// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "MurphysLawPlayerState.h"
#include <MurphysLaw/Interface/MurphysLawIController.h>
#include "Blueprint/UserWidget.h"
#include "MurphysLawPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MURPHYSLAW_API AMurphysLawPlayerController : public APlayerController, public IMurphysLawIController
{
	GENERATED_BODY()

	/** Sound to play each time we die */
	TArray<class USoundBase*> DeathSounds;
	TArray<class USoundBase*> KillSounds;

public:	
	AMurphysLawPlayerController();

	UFUNCTION(Reliable, Client)
	void PostLoginDone();

	UFUNCTION(BlueprintImplementableEvent, meta = (CallInEditor = "true"))
	void OnPostLoginDone();

	/** Starts the online game using the session name in the PlayerState */
	UFUNCTION(Reliable, Client)
	void ClientStartOnlineGame();

	UFUNCTION(Reliable, Client)
	void RefreshPlayerList();

	UFUNCTION(Reliable, Client)
	void OnMatchHasEnded();

	UFUNCTION(Reliable, Client)
	void OnOtherPlayerConnected(const FString& PlayerName);

	UFUNCTION(Reliable, Client)
	void OnOtherPlayerDisonnected(const FString& PlayerName);

	UFUNCTION(Reliable, Client)
	void OnSendDeathMessage(const FString& DeathMessage);

	UFUNCTION(Reliable, Client)
	// Event when controlled pawn has killed other player
	void OnKilledOther();

	UFUNCTION(BlueprintImplementableEvent, meta = (CallInEditor = "true"))
	void OnRefreshPlayerList();

	// Event when controlled pawn has died
	void OnKilled(const float TimeToRespawn) override;

	void Respawn() override;

	/** Called when the player asks for the In-Game menu */
	void ToggleInGameMenu();

	/** Reports the instance of the character */
	FORCEINLINE class AMurphysLawCharacter* GetMyCharacter() const { return MyCharacter; }

	/** Reports the instance of the HUD */
	FORCEINLINE class UMurphysLawHUDWidget* GetHUDInstance() const { return HUDInstance; }

	/** Shows the DamageIndicator on the HUD */
	void ShowDamage(float Angle);

	void PickedUpItem(class USoundBase* CollectSound);

	/** Empty function so that the base class doesn't destroy the pawn (the pawn is re-used by a AIController */
	void PawnLeavingGame() override;

	void ChangeHUDVisibility(ESlateVisibility visibility);
	
protected:
	/** Called when the pawn has been possessed */
	void BeginPlayingState() override;

	/** Called when the Player Controller is about to be destroyed */ 
	void EndPlayingState() override;

	void SetupInputComponent() override;

	/** The widget class to use for the HUD class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UMurphysLawHUDWidget> HUDWidgetClass;

	/** The instance of the HUD */
	UPROPERTY(BlueprintReadWrite, Category = "HUD")
	class UMurphysLawHUDWidget* HUDInstance;

	/** The type of the In-Game menu */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "InGameMenu", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UMurphysLawInGameMenu> InGameMenuClass;

	/** The instance of the In-Game menu */
	UPROPERTY()
	class UMurphysLawInGameMenu* InGameMenuInstance;

	/** The type of the Scoreboard */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Scoreboard", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UMurphysLawScoreboardWidget> ScoreboardClass;

	/** The instance of the Scoreboard */
	UPROPERTY()
	class UMurphysLawScoreboardWidget* ScoreboardInstance;

	/** Shows the DamageIndicator on the HUD of the clients */
	UFUNCTION(Reliable, Client)
	void Client_ShowDamage(float Angle);

private:
	/** Indicates whether the In-Game menu is open or not */
	bool IsInGameMenuOpen;

	/** Keeps an instance on the character this PlayerController controls */
	class AMurphysLawCharacter* MyCharacter;

	/** Handle for efficient management of ClientStartOnlineGame timer */
	FTimerHandle TimerHandle_ClientStartOnlineGame;

	/** Handle for efficient management of the Respawn timer */
	FTimerHandle TimerHandle_Respawn;

	/** Called to spawn all the UserWidgets of the player */
	void SpawnWidgets();

	/** Called when the player presses the 'Test' key */
	void OnTest();

	void InitSoundEffects();

#pragma region Input Callback functions

	/** Callbacks of the EquipWeapon keys */
	void EquipWeapon0();
	void EquipWeapon1();
	void EquipWeapon2();

	/** Callbacks of the Weapon action keys */
	void OnFire();
	void OnReload();
	void OnAim();
	void OnStopAiming();

	/** Callback of the Mouvement action keys */
	void OnCrouchToggle();
	void OnJump();
	void OnStopJumping();
	void OnMoveForward(float Value);
	void OnMoveRight(float Value);
	void OnRun();
	void OnStopRunning();

	/** Called when the player presses the key to show the scoreboard */
	void ShowScoreboard();

	/** Called when the player releases the key to show the scoreboard */
	void HideScoreboard();

#pragma endregion 
};

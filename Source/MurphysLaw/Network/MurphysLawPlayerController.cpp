// Fill out your copyright notice in the Description page of Project Settings.

#include "MurphysLaw.h"
#include "MurphysLawPlayerController.h"
#include "MurphysLawPlayerState.h"
#include "../Character/MurphysLawCharacter.h"
#include "MurphysLawGameState.h"
#include "MurphysLawGameInstance.h"
#include "MurphysLawGameMode.h"
#include "MurphysLawPlayerStart.h"
#include "../HUD/MurphysLawHUDWidget.h"
#include "../Menu/MurphysLawInGameMenu.h"
#include "../HUD/MurphysLawScoreboardWidget.h"
#include "../Utils/MurphysLawUtils.h"

AMurphysLawPlayerController::AMurphysLawPlayerController()
{
	InitSoundEffects();
}

void AMurphysLawPlayerController::PostLoginDone_Implementation()
{
		this->OnPostLoginDone();
}

/** Starts the online game using the session name in the PlayerState */
void AMurphysLawPlayerController::ClientStartOnlineGame_Implementation()
{
	AMurphysLawPlayerState* MurphysLawPlayerState = Cast<AMurphysLawPlayerState>(PlayerState);
	if (MurphysLawPlayerState)
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
			if (Sessions.IsValid())
			{
				ChangeHUDVisibility(ESlateVisibility::Visible);
				UE_LOG(LogOnline, Log, TEXT("Starting session %s on client"), *MurphysLawPlayerState->SessionName.ToString());
				// Starting the client session
				Sessions->StartSession(MurphysLawPlayerState->SessionName);
				UMurphysLawGameInstance* gameInstance = Cast<UMurphysLawGameInstance>(GetGameInstance());
				// Blueprint event, remove the lobby widget
				gameInstance->OnStartSessionCompleted();
			}
		}
	}
	else
	{
		// Keep retrying until player state is replicated
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_ClientStartOnlineGame, this, &AMurphysLawPlayerController::ClientStartOnlineGame_Implementation, 0.2f, false);
	}
}

void AMurphysLawPlayerController::ChangeHUDVisibility(ESlateVisibility visibility)
{
	if (HUDInstance)
		HUDInstance->SetVisibility(visibility);
}

void AMurphysLawPlayerController::RefreshPlayerList_Implementation()
{
	// Blueprint event, update the lobby
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AMurphysLawPlayerController::OnRefreshPlayerList, 1.5f, false);
}

void AMurphysLawPlayerController::OnKilled(const float TimeToRespawn)
{
	int32 index = FMath::RandRange(0, DeathSounds.Num() - 1);
	if (DeathSounds.IsValidIndex(index) && DeathSounds[index] != nullptr) 
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSounds[index], GetPawn()->GetActorLocation());
	}
	DisableInput(this);
	UnPossess();
	GetWorldTimerManager().SetTimer(TimerHandle_Respawn, this, &AMurphysLawPlayerController::Respawn, TimeToRespawn);
}

void AMurphysLawPlayerController::OnKilledOther_Implementation()
{
	int32 index = FMath::RandRange(0, KillSounds.Num() - 1);
	if (KillSounds.IsValidIndex(index) && KillSounds[index] != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, KillSounds[index], GetPawn()->GetActorLocation());
	}
}

void AMurphysLawPlayerController::Respawn()
{
	EnableInput(this);
	AMurphysLawGameMode* GameMode = Cast<AMurphysLawGameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		AMurphysLawPlayerState* MurphysLawPlayerState = Cast<AMurphysLawPlayerState>(PlayerState);
		MurphysLawUtils::RespawnCharacter(GameMode, MurphysLawPlayerState, MyCharacter);
		Possess(MyCharacter);
		ChangeHUDVisibility(ESlateVisibility::Visible);
	}
}

void AMurphysLawPlayerController::OnMatchHasEnded_Implementation()
{
	ChangeHUDVisibility(ESlateVisibility::Collapsed);

	if (IsInGameMenuOpen)
		ToggleInGameMenu();
	DisableInput(this);
	ShowScoreboard();
}

void AMurphysLawPlayerController::OnSendDeathMessage_Implementation(const FString& DeathMessage)
{	
	// Adds the message to the screen
	if (HUDInstance) HUDInstance->AddOnScreenMessage(DeathMessage);
}

void AMurphysLawPlayerController::OnOtherPlayerConnected_Implementation(const FString& PlayerName)
{
	if (HUDInstance)
		HUDInstance->AddOnScreenMessage(FString::Printf(TEXT("%s joined the game"), *PlayerName));
}

void AMurphysLawPlayerController::OnOtherPlayerDisonnected_Implementation(const FString& PlayerName)
{
	if(HUDInstance)
		HUDInstance->AddOnScreenMessage(FString::Printf(TEXT("%s left the game"), *PlayerName));
}

// Called when the pawn has been possessed
void AMurphysLawPlayerController::BeginPlayingState()
{
	APlayerController::BeginPlayingState();

	MyCharacter = Cast<AMurphysLawCharacter>(GetCharacter());

	checkf(MyCharacter != nullptr, TEXT("MyCharacter is null in MurphysLawPlayerController"));

	// Only done when the controller is local
	if (IsLocalPlayerController())
	{
		// At the spawn of the player, the in-game menu is hidden
		IsInGameMenuOpen = false;

		SpawnWidgets();
	}
}

void AMurphysLawPlayerController::InitSoundEffects()
{
	// Try to load the sound of switching weapon
	static ConstructorHelpers::FObjectFinder<USoundBase> DeathSound1(TEXT("SoundWave'/Game/MurphysLawRessources/SoundEffects/S_BeAManAndConfrontSomebody.S_BeAManAndConfrontSomebody'"));
	static ConstructorHelpers::FObjectFinder<USoundBase> DeathSound2(TEXT("SoundWave'/Game/MurphysLawRessources/SoundEffects/S_ImSickOfThis.S_ImSickOfThis'"));
	static ConstructorHelpers::FObjectFinder<USoundBase> DeathSound3(TEXT("SoundWave'/Game/MurphysLawRessources/SoundEffects/S_OldLameAssNigga.S_OldLameAssNigga'"));
	static ConstructorHelpers::FObjectFinder<USoundBase> DeathSound4(TEXT("SoundWave'/Game/MurphysLawRessources/SoundEffects/S_OhMyGodNoNo.S_OhMyGodNoNo'"));
	
	static ConstructorHelpers::FObjectFinder<USoundBase> KillSound1(TEXT("SoundWave'/Game/MurphysLawRessources/SoundEffects/S_LocalAssBitch.S_LocalAssBitch'"));
	static ConstructorHelpers::FObjectFinder<USoundBase> KillSound2(TEXT("SoundWave'/Game/MurphysLawRessources/SoundEffects/S_YeahBitchImAKiller.S_YeahBitchImAKiller'"));
	static ConstructorHelpers::FObjectFinder<USoundBase> KillSound3(TEXT("SoundWave'/Game/MurphysLawRessources/SoundEffects/S_YouAintGotNoDaddy.S_YouAintGotNoDaddy'"));

	if (DeathSound1.Succeeded()) DeathSounds.Add(DeathSound1.Object);
	if (DeathSound2.Succeeded()) DeathSounds.Add(DeathSound2.Object);
	if (DeathSound3.Succeeded()) DeathSounds.Add(DeathSound3.Object);
	if (DeathSound4.Succeeded()) DeathSounds.Add(DeathSound4.Object);

	if (KillSound1.Succeeded()) KillSounds.Add(KillSound1.Object);
	if (KillSound2.Succeeded()) KillSounds.Add(KillSound2.Object);
	if (KillSound3.Succeeded()) KillSounds.Add(KillSound3.Object);
}

void AMurphysLawPlayerController::SetupInputComponent()
{
	APlayerController::SetupInputComponent();

	// Dans un jeu multiplayer où le Controller peut changer de Pawn dynamiquement, 
	// c'est mieux de gérer les contrôles dans le Controller et de faire commander le Pawn
	// SOURCE : https://docs-origin.unrealengine.com/latest/INT/Gameplay/Framework/Controller/PlayerController/index.html

	InputComponent->BindAction("ShowInGameMenu", IE_Pressed, this, &AMurphysLawPlayerController::ToggleInGameMenu);
	InputComponent->BindAction("ShowScoreboard", IE_Pressed, this, &AMurphysLawPlayerController::ShowScoreboard);
	InputComponent->BindAction("ShowScoreboard", IE_Released, this, &AMurphysLawPlayerController::HideScoreboard);

	InputComponent->BindAction("EquipWeapon0", IE_Pressed, this, &AMurphysLawPlayerController::EquipWeapon0);
	InputComponent->BindAction("EquipWeapon1", IE_Pressed, this, &AMurphysLawPlayerController::EquipWeapon1);
	InputComponent->BindAction("EquipWeapon2", IE_Pressed, this, &AMurphysLawPlayerController::EquipWeapon2);

	InputComponent->BindAction("Fire", IE_Pressed, this, &AMurphysLawPlayerController::OnFire);
	InputComponent->BindAction("Reload", IE_Pressed, this, &AMurphysLawPlayerController::OnReload);	

	InputComponent->BindAction("Crouch", IE_Pressed, this, &AMurphysLawPlayerController::OnCrouchToggle);

	InputComponent->BindAction("Jump", IE_Pressed, this, &AMurphysLawPlayerController::OnJump);
	InputComponent->BindAction("Jump", IE_Released, this, &AMurphysLawPlayerController::OnStopJumping);

	InputComponent->BindAxis("MoveForward", this, &AMurphysLawPlayerController::OnMoveForward);
	InputComponent->BindAxis("MoveRight", this, &AMurphysLawPlayerController::OnMoveRight);

	InputComponent->BindAxis("Turn", this, &APlayerController::AddYawInput);
	InputComponent->BindAxis("LookUp", this, &APlayerController::AddPitchInput);

	InputComponent->BindAction("Run", IE_Pressed, this, &AMurphysLawPlayerController::OnRun);
	InputComponent->BindAction("Run", IE_Released, this, &AMurphysLawPlayerController::OnStopRunning);

	InputComponent->BindAction("Aim", IE_Pressed, this, &AMurphysLawPlayerController::OnAim);
	InputComponent->BindAction("Aim", IE_Released, this, &AMurphysLawPlayerController::OnStopAiming);
}

// Called when the Player Controller is about to be destroyed
void AMurphysLawPlayerController::EndPlayingState()
{
	APlayerController::EndPlayingState();

	// Only done when the controller is local
	if (IsLocalPlayerController())
	{
		// Hide the HUD of the player before he respawns
		ChangeHUDVisibility(ESlateVisibility::Collapsed);

		// Hide the InGameMenu of the player before he respawns
		if (InGameMenuInstance) InGameMenuInstance->SetVisibility(ESlateVisibility::Collapsed);

		// Show the Scoreboard of the player before he respawns
		if (ScoreboardInstance) ScoreboardInstance->Show();
	}
}

// Called when the player asks for the In-Game menu
void AMurphysLawPlayerController::ToggleInGameMenu()
{
	// The HUD widget and In-Game menu must be set because we toggle its visibility
	if (!HUDInstance || !InGameMenuInstance)
	{
		ShowError(TEXT("Can't toggle the In Game Menu : HUD or InGameMenu is null"));
		return;
	}

	IsInGameMenuOpen = !IsInGameMenuOpen;

	// Change the visibility of both the HUD and the menu
	InGameMenuInstance->SetVisibility(IsInGameMenuOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	ChangeHUDVisibility(IsInGameMenuOpen ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);

	// Change the input mode of the game
	if (IsInGameMenuOpen)
	{
		// To lock on the menu when it's visible
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(InGameMenuInstance->GetCachedWidget());
		InputMode.SetLockMouseToViewport(true);
		GetWorld()->GetFirstPlayerController()->SetInputMode(InputMode);
	}
	else
	{
		// Or to put back the input to normal when the menu is hidden
		FInputModeGameOnly InputMode;
		GetWorld()->GetFirstPlayerController()->SetInputMode(InputMode);
		FSlateApplication::Get().SetUserFocusToGameViewport(0);
		GetWorld()->GetFirstPlayerController()->bShowMouseCursor = false;
	}
}

// Called to spawn all the UserWidgets of the player
void AMurphysLawPlayerController::SpawnWidgets()
{
	// Check if the HUD type has been set
	if (HUDWidgetClass && !HUDInstance)
	{
		// We try to create a widget of HUDWidgetClass type
		HUDInstance = CreateWidget<UMurphysLawHUDWidget>(GetWorld(), HUDWidgetClass);
		if (HUDInstance)
		{
			// And if it is successfull, we add it to Viewport so the user sees it
			HUDInstance->AddToViewport();
		}
	}

	// Check if the In-Game menu type has been set
	if (InGameMenuClass && !InGameMenuInstance)
	{
		// Try to create the In-Game menu
		InGameMenuInstance = CreateWidget<UMurphysLawInGameMenu>(GetWorld(), InGameMenuClass);
		if (InGameMenuInstance)
		{
			// The menu starts hidden and is shown when the player presses the Escape key
			InGameMenuInstance->AddToViewport();
		}
	}

	// Check if the Scoreboard type has been set
	if (ScoreboardClass && !ScoreboardInstance)
	{
		// We try to create a widget of HUDWidgetClass type
		ScoreboardInstance = CreateWidget<UMurphysLawScoreboardWidget>(GetWorld(), ScoreboardClass);
		if (ScoreboardInstance)
		{
			// And if it is successfull, we add it to Viewport so the user sees it				
			ScoreboardInstance->AddToViewport();
		}
	}

	// When the HUD is created, we change its properties
	if (HUDInstance)
	{
		ChangeHUDVisibility(ESlateVisibility::Hidden);
		HUDInstance->SetPlayerController(this);
	}

	// When the InGameMenu is created, we change its properties
	if (InGameMenuInstance)
	{
		InGameMenuInstance->SetVisibility(ESlateVisibility::Collapsed);
		InGameMenuInstance->SetPlayerController(this);
	}

	// When the Scoreboard is created, we change its properties
	if (ScoreboardInstance)
	{
		ScoreboardInstance->Hide();
		ScoreboardInstance->SetPlayerController(this);
	}
}

// Callbacks of the EquipWeapon keys
void AMurphysLawPlayerController::EquipWeapon0() { MyCharacter->EquipWeapon(0); }
void AMurphysLawPlayerController::EquipWeapon1() { MyCharacter->EquipWeapon(1); }
void AMurphysLawPlayerController::EquipWeapon2() { MyCharacter->EquipWeapon(2); }

// Callbacks of the Weapon action keys
void AMurphysLawPlayerController::OnFire() { MyCharacter->Fire(); }
void AMurphysLawPlayerController::OnReload() { MyCharacter->Reload(); }
void AMurphysLawPlayerController::OnAim() { MyCharacter->Aim(); }
void AMurphysLawPlayerController::OnStopAiming() { MyCharacter->StopAiming(); }

// Callback of the Mouvement action keys
void AMurphysLawPlayerController::OnCrouchToggle() { MyCharacter->ToggleCrouch(); }
void AMurphysLawPlayerController::OnJump() { MyCharacter->Jump(); }
void AMurphysLawPlayerController::OnStopJumping() { MyCharacter->StopJumping(); }
void AMurphysLawPlayerController::OnMoveForward(float Value) { MyCharacter->MoveForward(Value); }
void AMurphysLawPlayerController::OnMoveRight(float Value) { MyCharacter->MoveRight(Value); }
void AMurphysLawPlayerController::OnRun() { MyCharacter->Run(); }
void AMurphysLawPlayerController::OnStopRunning() { MyCharacter->StopRunning(); }

// Called when the player presses the key to show the scoreboard
void AMurphysLawPlayerController::ShowScoreboard()
{
	if (ScoreboardInstance) ScoreboardInstance->Show();
}

// Called when the player releases the key to show the scoreboard
void AMurphysLawPlayerController::HideScoreboard()
{
	if (ScoreboardInstance) ScoreboardInstance->Hide();
}

/** Called when the player presses the 'Test' key */
void AMurphysLawPlayerController::OnTest()
{
	const char* SEPERATOR = "-------------------------------------------------";
	//ShowMessage(SEPERATOR, FColor::Magenta);
	// ShowMessage(, FColor::Magenta);
	// Show things that you need to see to debug the game
	//ShowMessage(SEPERATOR, FColor::Magenta);
}

// Shows the DamageIndicator on the HUD
void AMurphysLawPlayerController::ShowDamage(float Angle)
{
	if (IsLocalPlayerController())
	{
		Client_ShowDamage_Implementation(Angle);
	}
	else
	{
		Client_ShowDamage(Angle);
	}
}

// Shows the DamageIndicator on the HUD of the clients
void AMurphysLawPlayerController::Client_ShowDamage_Implementation(float Angle)
{
	// If the PlayerController has a HUD, we show damages on it
	if (HUDInstance != nullptr)
	{
		HUDInstance->ShowDamage(Angle);
	}
}

void AMurphysLawPlayerController::PickedUpItem(class USoundBase* CollectSound)
{
	/*if (IsLocalPlayerController() && CollectSound)
	{
		auto SomeSoundAC = UGameplayStatics::PlaySoundAttached(CollectSound, GetRootComponent());
	}*/
	UGameplayStatics::PlaySoundAtLocation(this, CollectSound, GetPawn()->GetActorLocation());

	auto PickedUpSound = UGameplayStatics::SpawnSoundAttached(CollectSound, GetRootComponent(),
		NAME_None, FVector(ForceInit), EAttachLocation::KeepRelativeOffset, true);
	PickedUpSound->SetVolumeMultiplier(0.0f);
}

/** Empty function so that the base class doesn't destroy the pawn (the pawn is re-used by a AIController */
void AMurphysLawPlayerController::PawnLeavingGame()
{
}
// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "MurphysLaw.h"
#include "MurphysLawGameMode.h"
#include "MurphysLawGameState.h"
#include "MurphysLawPlayerController.h"
#include "MurphysLawPlayerStart.h"
#include "MurphysLawPlayerState.h"
#include "../MurphysLawGameInstance.h"
#include <MurphysLaw/Character/MurphysLawCharacter.h>
#include <MurphysLaw/Settings/Teams/MurphysLawTeamColor.h>
#include <MurphysLaw/AI/MurphysLawAIController.h>
#include <MurphysLaw/Utils/MurphysLawUtils.h>
#include "GameFramework/Pawn.h"

AMurphysLawGameMode::AMurphysLawGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/MurphysLaw/Visual/Characters/Cowboy1/Partial/BP_Cowboy1_arms"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
		
	GameStateClass = AMurphysLawGameState::StaticClass();
	PlayerStateClass = AMurphysLawPlayerState::StaticClass();
	InactivePlayerStateLifeSpan = 0.f;
}

/** Initialize the game. This is called before actors' PreInitializeComponents. */
void AMurphysLawGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	
	// Save settings for player state access
	GameSettings = MurphysLawGameSettings::Parse(Options);

	InitTeamSpawnPointsPools();
	InitTeamCharacterPools();
}

AActor* AMurphysLawGameMode::ChoosePlayerStart(int32 TeamNum)
{
	checkf(TeamSpawnPoints.Contains(TeamNum), TEXT("Invalid team number : %i"), TeamNum);
	SpawnPointListInfo& Info = TeamSpawnPoints[TeamNum];
	checkf(Info.Locations.Num() > 0, TEXT("No spawn point detected for team %i"), TeamNum)

	// Simple round robin
	auto PlayerStart = Info.Locations[Info.LastIndexUsed];
	Info.LastIndexUsed = (Info.LastIndexUsed + 1 >= Info.Locations.Num()) ? 0 : Info.LastIndexUsed + 1;

	return PlayerStart;
}

void AMurphysLawGameMode::DefaultTimer()
{
	AMurphysLawGameState* const MyGameState = Cast<AMurphysLawGameState>(GameState);
	if (MyGameState && MyGameState->RemainingTime > 0)
	{
		MyGameState->RemainingTime--;		
		if (MyGameState->RemainingTime == 0)
		{
			if (MyGameState->MurphysLawMatchState == MurphysLawMatchState::EWarmUp)
			{
				ResetAllCharacters();
				MyGameState->ResetStats();
				UpdateMatchState(MurphysLawMatchState::EPlaying, GameSettings.GameTime);
			}
			else if (MyGameState->MurphysLawMatchState == MurphysLawMatchState::EPlaying)
				ProcessEndGame();
			else if (MyGameState->MurphysLawMatchState == MurphysLawMatchState::EScoreBoard)
			{
				UMurphysLawGameInstance* GameInstance = Cast<UMurphysLawGameInstance>(GetWorld()->GetGameInstance());
				if (GameInstance)
					GameInstance->DestroySessionAndLeaveGame();
			}
		}
	}
}

void AMurphysLawGameMode::ResetAllCharacters()
{
	// Restart bots and players 
	for (TActorIterator<AController> It(GetWorld()); It; ++It)
	{
		IMurphysLawIController* PlayerController = Cast<IMurphysLawIController>(*It);
		if (PlayerController != nullptr)
		{
			PlayerController->Respawn();
			AMurphysLawPlayerState* PlayerState = Cast<AMurphysLawPlayerState>(It->PlayerState);
			if (PlayerState)
				PlayerState->ResetStats();
		}
	}
}

void AMurphysLawGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// If there is really a new player
	if (NewPlayer != nullptr)
	{
		// Places the new player on a team before Super
		AMurphysLawPlayerState* NewPlayerState = CastChecked<AMurphysLawPlayerState>(NewPlayer->PlayerState);

		// If the new player state is valid
		if (NewPlayerState != nullptr && GameState != nullptr)
		{
			
			const int32 SelectedTeamId = GetBestTeamForNewPlayer(NewPlayer->PlayerState);
			NewPlayerState->SetTeam(SelectedTeamId);
			if (NewPlayer->GetPawn())
				NewPlayer->GetPawn()->Destroy();
				
			APawn* Pawn = GetAvailableAIControlledCharacter(SelectedTeamId);
			if (Pawn)
				NewPlayer->Possess(Pawn);

			NewPlayerState->SetActive(true);
		}

		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			AMurphysLawPlayerController* PC = Cast<AMurphysLawPlayerController>(*It);
			if (PC != nullptr)
			{
				if (PC == NewPlayer)
				{
					if (HasMatchStarted())
						PC->ClientStartOnlineGame();
					else
					{
						PC->PostLoginDone();
					}
				}
				else
				{
					AMurphysLawGameState* const MyGameState = Cast<AMurphysLawGameState>(GameState);
					if (MyGameState->MurphysLawMatchState == MurphysLawMatchState::EInLobby)
						PC->RefreshPlayerList();
					else
						PC->OnOtherPlayerConnected(NewPlayer->GetHumanReadableName());
				}
			}
		}
	}
}

void AMurphysLawGameMode::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();
	AMurphysLawGameState* const MyGameState = Cast<AMurphysLawGameState>(GameState);
	if(MyGameState)
		MyGameState->WinningTeam = DRAW;
	UpdateMatchState(MurphysLawMatchState::EInLobby, 0);
}

void AMurphysLawGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	GetWorldTimerManager().SetTimer(TimerHandle_DefaultTimer, this, &AMurphysLawGameMode::DefaultTimer, GetWorldSettings()->GetEffectiveTimeDilation(), true);
	
	if(GameSettings.WarmupWanted)
		UpdateMatchState(MurphysLawMatchState::EWarmUp, WARMUP_TIME);
	else
		UpdateMatchState(MurphysLawMatchState::EPlaying, GameSettings.GameTime);

	for (TActorIterator<AController> It(GetWorld()); It; ++It)
	{
		AMurphysLawAIController* PC = Cast<AMurphysLawAIController>(*It);
		if (PC)
			PC->SetBlackboardCanMove(true);
	}
}

bool AMurphysLawGameMode::ReadyToStartMatch_Implementation()
{
	return 	MatchState == MatchState::InProgress;
}

APlayerController* AMurphysLawGameMode::Login(class UPlayer * NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const TSharedPtr<const FUniqueNetId>& UniqueId, FString& ErrorMessage)
{
	MurphysLawGameSettings GameSettings = MurphysLawGameSettings::Parse(Options);

	APlayerController* PC = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);
	AMurphysLawPlayerController* MurphysLawPC = Cast<AMurphysLawPlayerController>(PC);

	if (MurphysLawPC)
	{
		MurphysLawPC->SetName(GameSettings.CharacterName);
	}
	return MurphysLawPC;
}

void AMurphysLawGameMode::HandleMatchHasEnded()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AMurphysLawPlayerController* PC = Cast<AMurphysLawPlayerController>(*It);
		if (PC)
			PC->OnMatchHasEnded();
		else
		{
			AMurphysLawAIController* PC = Cast<AMurphysLawAIController>(*It);
			if (PC)
				PC->SetBlackboardCanMove(false);
		}
	}
}

void AMurphysLawGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	
	if (UGameplayStatics::GetPlayerController(GetWorld(), 0) != Exiting)
	{
	ReturnCharaterToAI(Exiting);

		GameState->RemovePlayerState(Exiting->PlayerState);

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AMurphysLawPlayerController* PC = Cast<AMurphysLawPlayerController>(*It);
		if (PC)
		{
			if (PC != Exiting)
			{
				AMurphysLawGameState* const MyGameState = Cast<AMurphysLawGameState>(GameState);
				if (MyGameState->MurphysLawMatchState == MurphysLawMatchState::EInLobby)
				PC->RefreshPlayerList();
				else
					PC->OnOtherPlayerDisonnected(Exiting->GetHumanReadableName());
			}
		}
	}
	}
}


#pragma region Teams

// Assign a team for a player 
int32 AMurphysLawGameMode::GetBestTeamForNewPlayer(APlayerState* NewPlayerState) const
{
	// A better algorithmn could be to select the team that has the highest number
	// of available characters, which means the least players.
	// However, before that, the user hosting the game must not create its own character.
	// He must use the available pools !
	TArray<uint32> NbPlayersPerTeam;
	NbPlayersPerTeam.AddZeroed(GameSettings.NbTeams);

	// Counts how many are players there are in each team
	for (APlayerState* It : GameState->PlayerArray)
	{
		AMurphysLawPlayerState* IteratedPlayerState = Cast<AMurphysLawPlayerState>(It);
		if (IteratedPlayerState && NbPlayersPerTeam.IsValidIndex(IteratedPlayerState->GetTeam()) &&
			It != NewPlayerState)
		{
			++NbPlayersPerTeam[IteratedPlayerState->GetTeam()];
		}
	}
	// Identify team with least members
	int32 MinTeamId = 0;
	(void) FMath::Min(NbPlayersPerTeam, &MinTeamId);
	return MinTeamId;
}

// Get available character to possess
APawn* AMurphysLawGameMode::GetAvailableAIControlledCharacter(const int32 TeamId)
{
	APawn* SelectedCharacter = nullptr;
	if (TeamCharacterPool.Contains(TeamId) && TeamCharacterPool[TeamId].Num() > 0)
	{
		// Get a free reference
		const int32 SelectedCharacterIndex = FMath::RandRange(0, TeamCharacterPool[TeamId].Num() - 1);
		SelectedCharacter = TeamCharacterPool[TeamId][SelectedCharacterIndex];
		if (SelectedCharacter)
		{
			AMurphysLawPlayerState* PlayerState = Cast<AMurphysLawPlayerState>(SelectedCharacter->GetController()->PlayerState);
			PlayerState->SetActive(false);
			AIControllerPool[TeamId].Add(SelectedCharacter->GetController());
			TeamCharacterPool[TeamId].RemoveAt(SelectedCharacterIndex);
		}
		else
			ShowError("SelectedCharacter nullptr in GetAvailableAIControlledCharacter");
	}
	else
		ShowError("Not enough available players for team-" + FString::FromInt(TeamId));

	return SelectedCharacter;
}

// Get available character to possess
AController* AMurphysLawGameMode::ReturnCharaterToAI(AController* Exiting)
{
	AMurphysLawPlayerState* PlayerState = Cast<AMurphysLawPlayerState>(Exiting->PlayerState);
	AController* SelectedAIController = nullptr;
	if (PlayerState)
	{
		int32 TeamId = PlayerState->GetTeam();
		if (PlayerState && AIControllerPool.Contains(TeamId) && AIControllerPool[TeamId].Num() > 0)
		{
			// Get a free reference
			const int32 SelectedControllerIndex = FMath::RandRange(0, AIControllerPool[TeamId].Num() - 1);
			SelectedAIController = AIControllerPool[TeamId][SelectedControllerIndex];

			if (SelectedAIController)
			{
				//Get a reference on the pawn
				APawn* Pawn = Exiting->GetPawn();
				if (Pawn)
				{
					// Unpossess the pawn before let the AIController possess it
					Exiting->UnPossess();
					SelectedAIController->Possess(Pawn);

					TeamCharacterPool[TeamId].Add(Pawn);
					AIControllerPool[TeamId].RemoveAt(SelectedControllerIndex);

					AMurphysLawPlayerState* AIPlayerState = Cast<AMurphysLawPlayerState>(SelectedAIController->PlayerState);
					AIPlayerState->SetActive(true);
				}
				else
					ShowError("Exiting->GetPawn() nullptr in ReturnCharaterToAI");
			}
			else
				ShowError("SelectedAIController nullptr in ReturnCharaterToAI");
		}
		else
			ShowError("Not enough available players for team " + FString::FromInt(TeamId));
	}

	return SelectedAIController;
}

// Store an available character
void AMurphysLawGameMode::AddCharacterForAIControl(const int32 TeamId, APawn* ReleasedCharacter)
{
	if (ReleasedCharacter && TeamCharacterPool.Contains(TeamId))
	{		
		// Store the reference
		TeamCharacterPool[TeamId].Add(ReleasedCharacter);
	}
	else
	{
		ShowError("Invalid character released for reuse for team" + FString::FromInt(TeamId));
	}
}

// Creates the unpossessed characters for each teams */
void AMurphysLawGameMode::InitTeamCharacterPools()
{
	const TArray<MurphysLawTeamColor> TeamColors = MurphysLawTeamColor::GetPredefinedColors(GameSettings.NbTeams);

	int32 AIId = 0;
	FString AIName = "Bot ";
	for(int32 TeamId = 0; TeamId < GameSettings.NbTeams; ++TeamId)
	{
		// Initialize array
		TeamCharacterPool.Add(TeamId, TArray<APawn*>());
		AIControllerPool.Add(TeamId, TArray<AController*>());

		for (int32 PlayerId = 0; PlayerId < GameSettings.NbPlayersPerTeam; ++PlayerId)
		{
			// Create the character and set its team color tint
			AMurphysLawCharacter* const NewCharacter = CreateCharacter();
			NewCharacter->SetMeshTeamColorTint(TeamColors[TeamId]);

			// Set the team of this newly created character
			if (NewCharacter && NewCharacter->GetController() && NewCharacter->GetController()->PlayerState)
			{
				AMurphysLawPlayerState* NewPlayerState = Cast<AMurphysLawPlayerState>(NewCharacter->GetController()->PlayerState);
				if (NewPlayerState)
				{
					NewPlayerState->SetTeam(TeamId);
					NewPlayerState->SetActive(true);
					NewPlayerState->SetPlayerName(AIName + FString::FromInt(AIId));
					AIId++;
				}
			}

			auto StartPoint = ChoosePlayerStart(TeamId);
			NewCharacter->SetActorLocation(StartPoint->GetActorLocation());

			// Save the references
			AddCharacterForAIControl(TeamId, NewCharacter);
		}
	}
}

// Creates the list of available spawn points per team
void AMurphysLawGameMode::InitTeamSpawnPointsPools()
{
	// Prepare pool of spawn points
	for (int32 TeamId = 0; TeamId < GameSettings.NbTeams; ++TeamId)
	{
		SpawnPointListInfo TeamSpawnInfo{};
		TeamSpawnInfo.LastIndexUsed = 0;
		TeamSpawnPoints.Add(TeamId, TeamSpawnInfo);
	}

	// Detect spawn points from map
	for (TActorIterator<AMurphysLawPlayerStart> StartIt(GetWorld()); StartIt; ++StartIt)
		TeamSpawnPoints[StartIt->Team].Locations.Add(*StartIt);
}

#pragma endregion


AMurphysLawCharacter* AMurphysLawGameMode::CreateCharacter() const
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = nullptr;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.OverrideLevel = nullptr;

	// Create the character
	// One of it's base class must be MurphysLawCharacter
	APawn* NewPawn = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, SpawnParams);
	return CastChecked<AMurphysLawCharacter>(NewPawn);
}

#pragma endregion

void AMurphysLawGameMode::SendDeathMessage(class AMurphysLawPlayerController* Killer, FString DeathMessage)
{
	// Only sent to player
	if(Killer != nullptr) Killer->OnKilledOther();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AMurphysLawPlayerController* PC = Cast<AMurphysLawPlayerController>(*It);
		if (PC)
		{
			PC->OnSendDeathMessage(DeathMessage);
		}
	}
	AMurphysLawGameState* const MyGameState = Cast<AMurphysLawGameState>(GameState);
	if (MyGameState)
	{
		MyGameState->WinningTeam = (MyGameState->ScoreTeamA == MyGameState->ScoreTeamB ? DRAW : (MyGameState->ScoreTeamA > MyGameState->ScoreTeamB ? TEAM_A : TEAM_B));
		if (GameSettings.NbPointsForWin <= MyGameState->ScoreTeamA || GameSettings.NbPointsForWin <= MyGameState->ScoreTeamB)
			ProcessEndGame();
	}
}

void AMurphysLawGameMode::ProcessEndGame()
{
	AMurphysLawGameState* const MyGameState = Cast<AMurphysLawGameState>(GameState);
	if (MyGameState)
	{
		UpdateMatchState(MurphysLawMatchState::EScoreBoard, SCOREBOARD_TIME);
		EndMatch();
	}
}

void AMurphysLawGameMode::UpdateMatchState(MurphysLawMatchState State, int32 RemainingTime)
{
	AMurphysLawGameState* const MyGameState = Cast<AMurphysLawGameState>(GameState);
	if (MyGameState)
	{
		MyGameState->MurphysLawMatchState = State;
		MyGameState->RemainingTime = RemainingTime;
	}
}
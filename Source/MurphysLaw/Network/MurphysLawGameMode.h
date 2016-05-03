// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameMode.h"
#include "../Settings/MurphysLawGameSettings.h"
#include "MurphysLawGameState.h"
#include "MurphysLawGameMode.generated.h"


struct SpawnPointListInfo
{
	// The available locations
	TArray<class AMurphysLawPlayerStart*> Locations;

	/** The last location used.
		Act as a simple round-robin index to avoid using the
		same spawn point for two players, which causes collisions
	*/
	int32 LastIndexUsed;
};

UCLASS(minimalapi)
class AMurphysLawGameMode : public AGameMode
{
	GENERATED_BODY()

	static const uint32 WARMUP_TIME = 30;
	static const uint32 SCOREBOARD_TIME = 5;

	/** Handle for efficient management of DefaultTimer timer */
	FTimerHandle TimerHandle_DefaultTimer;

	/** The selected options for the game */
	MurphysLawGameSettings GameSettings;

	/** The unpossessed characters for each team */
	TMap<int32, TArray<class APawn*> > TeamCharacterPool;

	/** The unpossessed controllers for each team */
	TMap<int32, TArray<class AController*> > AIControllerPool;

	/** The available spawn points for each team */
	TMap<int32, SpawnPointListInfo> TeamSpawnPoints;

public: 
	static const uint8 TEAM_A = 0;
	static const uint8 TEAM_B = 1;
	static const uint8 DRAW = 2;

private:
	/** Creates the unpossessed characters for each teams */
	void InitTeamCharacterPools();

	/** Creates the list of available spawn points per team */
	void InitTeamSpawnPointsPools();

	class AMurphysLawCharacter* CreateCharacter() const;

	/** Initialize the game. This is called before actors' PreInitializeComponents. */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	/** update remaining time */
	virtual void DefaultTimer();

	bool ShouldSpawnAtStartSpot(AController* Player) override { return false; };

	void ProcessEndGame();

	void UpdateMatchState(MurphysLawMatchState State, int32 RemainingTime);

protected:
	/** Assign a team for a player */
	int32 GetBestTeamForNewPlayer(APlayerState* NewPlayerState) const;

	/** Resets the position, inventory and ammos of all characters */
	void ResetAllCharacters();

public:
	AMurphysLawGameMode();

	AActor* ChoosePlayerStart(int32 TeamNum);

	/** called before startmatch */
	virtual void HandleMatchIsWaitingToStart() override;

	// Called when the state transitions to InProgress
	virtual void HandleMatchHasStarted() override;

	// Called when the map transitions to WaitingPostMatch
	virtual void HandleMatchHasEnded() override;

	UFUNCTION(BlueprintCallable, Category = "GameMode")
	bool ReadyToStartMatch_Implementation();

	virtual APlayerController* Login(class UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const TSharedPtr<const FUniqueNetId>& UniqueId, FString& ErrorMessage) override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	/** Get a character controlled by an AI to take its place.
	This will mostly happen when a player will join the game. */
	class APawn* GetAvailableAIControlledCharacter(const int32 TeamId);

	AController* ReturnCharaterToAI(AController* Exiting);

	/** Release a character and let it be controlled by an AI.
	This will mostly happen if a player leaves an in progress game. */
	void AddCharacterForAIControl(const int32 TeamId, class APawn* ReleasedCharacter);

	void SendDeathMessage(class AMurphysLawPlayerController* Killer, FString DeathMessage);
};




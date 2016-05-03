// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerState.h"
#include "MurphysLawPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class MURPHYSLAW_API AMurphysLawPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	bool IsActive() const;
	void SetActive(bool Active);

#pragma region Team properties and methods
	
public:
	/** Accessor function for the team */
	int32 GetTeam() const;

	/** Sets the new team of the player */
	void SetTeam(int32 NewTeam);

protected:
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "PlayerState")
	int32 Team;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "PlayerState")
	bool Active = false;

#pragma endregion

#pragma region Kills properties and methods

public:
	/** Accessor function for the number of kills of the player */
	int32 GetNbKills() const;

	/** Increments the number of kills of the player */
	void IncrementNbKills();

	/*------------- Replication functions for the NbKills property -------------*/
	/** Method called by the clients to update their number of kills */
	UFUNCTION(Reliable, Server, WithValidation)
	void Server_IncrementNbKills();

	/** Called on the client when NbKills property is changed by the server */
	UFUNCTION()
	void OnRep_IncrementNbKills();

protected:
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_IncrementNbKills, BlueprintReadWrite, Category = "PlayerState")
	int32 NbKills;

#pragma endregion

#pragma region Deaths properties and methods

public:
	/** Accessor function for the number of deaths of the player */
	int32 GetNbDeaths() const;

	/** Increments the number of deaths of the player */
	void IncrementNbDeaths();

	void ResetStats();

	/*------------- Replication functions for the NbDeaths property -------------*/
	/** Method called by the clients to update their number of kills */
	UFUNCTION(Reliable, Server, WithValidation)
	void Server_IncrementNbDeaths();

	/** Called on the client when NbKills property is changed by the server */
	UFUNCTION()
	void OnRep_IncrementNbDeaths();

protected:
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_IncrementNbDeaths, BlueprintReadWrite, Category = "PlayerState")
	int32 NbDeaths;

#pragma endregion
};

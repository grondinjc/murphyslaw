// Fill out your copyright notice in the Description page of Project Settings.

#include "MurphysLaw.h"
#include "MurphysLawPlayerState.h"
#include "UnrealNetwork.h"

// Indicates to the server what properties of the object to replicate on the clients
void AMurphysLawPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMurphysLawPlayerState, Team);
	DOREPLIFETIME(AMurphysLawPlayerState, NbKills);
	DOREPLIFETIME(AMurphysLawPlayerState, NbDeaths);
	DOREPLIFETIME(AMurphysLawPlayerState, Active);
}

#pragma region Team methods

// Accessor function for the team
int32 AMurphysLawPlayerState::GetTeam() const
{
	return Team;
}

// Sets the new team of the player
void AMurphysLawPlayerState::SetTeam(int32 NewTeam)
{
	Team = NewTeam;
}

#pragma endregion

#pragma region Kills methods

// Accessor function for the number of kills of the player
int32 AMurphysLawPlayerState::GetNbKills() const
{
	return NbKills;
}

// Increments the number of kills of the player
void AMurphysLawPlayerState::IncrementNbKills()
{
	// If the server modifies the value of the property, it is sent to the clients automatically
	if (Role == ROLE_Authority)
	{
		Server_IncrementNbKills_Implementation();
	}
	else
	{
		// If a client wants to modify the value, it has to send a request to the server 
		Server_IncrementNbKills();
	}
}

// Functions that do the validation and implementation of the IncrementNbKills
bool AMurphysLawPlayerState::Server_IncrementNbKills_Validate() { return true; }
void AMurphysLawPlayerState::Server_IncrementNbKills_Implementation()
{
	NbKills++;
}

// Called on the client when NbKills property is changed by the server
void AMurphysLawPlayerState::OnRep_IncrementNbKills()
{
	// do nothing, because the number of kills doesn't affect anything in the UI yet
}

#pragma endregion

#pragma region Deaths methods

// Accessor function for the number of deaths of the player
int32 AMurphysLawPlayerState::GetNbDeaths() const
{
	return NbDeaths;
}

// Increments the number of deaths of the player
void AMurphysLawPlayerState::IncrementNbDeaths()
{
	// If the server modifies the value of the property, it is sent to the clients automatically
	if (Role == ROLE_Authority)
	{
		Server_IncrementNbDeaths_Implementation();
	}
	else
	{
		// If a client wants to modify the value, it has to send a request to the server 
		Server_IncrementNbDeaths();
	}
}

// Functions that do the validation and implementation of the IncrementNbDeaths
bool AMurphysLawPlayerState::Server_IncrementNbDeaths_Validate() { return true; }
void AMurphysLawPlayerState::Server_IncrementNbDeaths_Implementation()
{
	NbDeaths++;
}

// Called on the client when NbDeaths property is changed by the server
void AMurphysLawPlayerState::OnRep_IncrementNbDeaths()
{
	// do nothing, because the number of deaths doesn't affect anything in the UI yet
}

#pragma endregion

bool AMurphysLawPlayerState::IsActive() const
{
	return Active;
}

void AMurphysLawPlayerState::SetActive(bool Active)
{
	this->Active = Active;
}

void AMurphysLawPlayerState::ResetStats()
{
	NbKills = 0;
	NbDeaths = 0;
}
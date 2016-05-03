// Fill out your copyright notice in the Description page of Project Settings.

#include "MurphysLaw.h"
#include "MurphysLawGameSettings.h"
#include "../Utils/MurphysLawUtils.h"


// Constants definition of option names
const FString MurphysLawGameSettings::OPT_GAME_TIME(TEXT("GameTime"));
const FString MurphysLawGameSettings::OPT_WARMUP_WANTED(TEXT("WarmupWanted"));
const FString MurphysLawGameSettings::OPT_NB_POINTS_FOR_WIN(TEXT("NbPointsForWin"));
const FString MurphysLawGameSettings::OPT_NB_TEAMS(TEXT("NbTeams"));
const FString MurphysLawGameSettings::OPT_NB_PLAYERS_PER_TEAM(TEXT("NbPlayersPerTeams"));
const FString MurphysLawGameSettings::OPT_CHARACTER_NAME(TEXT("CharacterName"));



// Constants definition of defaults values
const int32 MurphysLawGameSettings::DEFAULT_GAME_TIME(300);
const bool MurphysLawGameSettings::DEFAULT_WARMUP_WANTED(true);
const int32 MurphysLawGameSettings::DEFAULT_NB_POINTS_FOR_WIN(500);
const int32 MurphysLawGameSettings::DEFAULT_NB_TEAMS(2);
const int32 MurphysLawGameSettings::DEFAULT_NB_PLAYERS_PER_TEAM(4);
const FString MurphysLawGameSettings::DEFAULT_CHARACTER_NAME("Cowboy_");
const uint32 MurphysLawGameSettings::DEFAULT_CHARACTER_NAME_MIN_ID(1);
const uint32 MurphysLawGameSettings::DEFAULT_CHARACTER_NAME_MAX_ID(999);

// Default constructor
MurphysLawGameSettings::MurphysLawGameSettings() :
	GameTime{ DEFAULT_GAME_TIME },
	WarmupWanted{DEFAULT_WARMUP_WANTED },
	NbPointsForWin{ DEFAULT_NB_POINTS_FOR_WIN }, 
	NbTeams{ DEFAULT_NB_TEAMS} ,
	NbPlayersPerTeam{ DEFAULT_NB_PLAYERS_PER_TEAM },
	CharacterName{ DEFAULT_CHARACTER_NAME + FString::FromInt(FMath::RandRange(DEFAULT_CHARACTER_NAME_MIN_ID, DEFAULT_CHARACTER_NAME_MAX_ID)) }
{}

MurphysLawGameSettings::~MurphysLawGameSettings()
{
}

// Marshalling function that includes leading separator
FString MurphysLawGameSettings::Serialize() const
{
	static const auto ParamFormat = TEXT("%s=%s");
	TArray<FString> QueryParams;

	// GameTimeParam
	QueryParams.Add(FString::Printf(ParamFormat, *OPT_GAME_TIME, *MurphysLawUtils::IntToString(GameTime)));
	// WarmupWantedParam
	QueryParams.Add(FString::Printf(ParamFormat, *OPT_WARMUP_WANTED, *MurphysLawUtils::BoolToString(WarmupWanted)));
	// NbPointForWinParam
	QueryParams.Add(FString::Printf(ParamFormat, *OPT_NB_POINTS_FOR_WIN, *MurphysLawUtils::IntToString(NbPointsForWin)));
	// NbTeamsParam
	QueryParams.Add(FString::Printf(ParamFormat, *OPT_NB_TEAMS, *MurphysLawUtils::IntToString(NbTeams)));
	// NbPlayersPerTeamParam
	QueryParams.Add(FString::Printf(ParamFormat, *OPT_NB_PLAYERS_PER_TEAM, *MurphysLawUtils::IntToString(NbPlayersPerTeam)));
	// CharacterNameParam
	QueryParams.Add(FString::Printf(ParamFormat, *OPT_CHARACTER_NAME, *CharacterName));

	// Build the parameters string
	static const auto ParamSeparator = TEXT("?");
	return ParamSeparator + FString::Join(QueryParams, ParamSeparator);
}

// Unmarshalling function
MurphysLawGameSettings MurphysLawGameSettings::Parse(const FString& DataUrl)
{
	const FString GameTimeStr = UGameplayStatics::ParseOption(DataUrl, OPT_GAME_TIME);
	const FString WarmupWantedStr = UGameplayStatics::ParseOption(DataUrl, OPT_WARMUP_WANTED);
	const FString NbPointsForWinStr = UGameplayStatics::ParseOption(DataUrl, OPT_NB_POINTS_FOR_WIN);
	const FString NbTeamStr = UGameplayStatics::ParseOption(DataUrl, OPT_NB_TEAMS);
	const FString NbPlayersPerTeamStr = UGameplayStatics::ParseOption(DataUrl, OPT_NB_PLAYERS_PER_TEAM);
	const FString CharacterNameStr = UGameplayStatics::ParseOption(DataUrl, OPT_CHARACTER_NAME);

	MurphysLawGameSettings Settings;
	Settings.GameTime = GameTimeStr.IsEmpty() ? DEFAULT_GAME_TIME : MurphysLawUtils::StringToInt(GameTimeStr);
	Settings.WarmupWanted = WarmupWantedStr.IsEmpty() ? DEFAULT_WARMUP_WANTED : MurphysLawUtils::StringToBool(WarmupWantedStr);
	Settings.NbPointsForWin = NbPointsForWinStr.IsEmpty() ? DEFAULT_NB_POINTS_FOR_WIN : MurphysLawUtils::StringToInt(NbPointsForWinStr);
	Settings.NbTeams = NbTeamStr.IsEmpty() ? DEFAULT_NB_TEAMS : MurphysLawUtils::StringToInt(NbTeamStr);
	Settings.NbPlayersPerTeam = NbPlayersPerTeamStr.IsEmpty() ? DEFAULT_NB_PLAYERS_PER_TEAM : MurphysLawUtils::StringToInt(NbPlayersPerTeamStr);
	Settings.CharacterName = CharacterNameStr.IsEmpty() ? DEFAULT_CHARACTER_NAME + FString::FromInt(FMath::RandRange(DEFAULT_CHARACTER_NAME_MIN_ID, DEFAULT_CHARACTER_NAME_MAX_ID)) : CharacterNameStr;

	return Settings;
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/**
 * Holds the settings of a game such as the number of players per teams,
 * the numbers of teams, the name of the map ...
 * 
 * Those are the settings that a player would choose through a menu.
 */
class MURPHYSLAW_API MurphysLawGameSettings
{
#pragma region Constants
private:
	static const FString OPT_GAME_TIME;
	static const int32 DEFAULT_GAME_TIME;

	static const FString OPT_WARMUP_WANTED;
	static const bool DEFAULT_WARMUP_WANTED;

	static const FString OPT_NB_POINTS_FOR_WIN;
	static const int32 DEFAULT_NB_POINTS_FOR_WIN;

	static const FString OPT_NB_TEAMS;
	static const int32 DEFAULT_NB_TEAMS;

	static const FString OPT_NB_PLAYERS_PER_TEAM;
	static const int32 DEFAULT_NB_PLAYERS_PER_TEAM;

	static const FString OPT_CHARACTER_NAME;
	static const FString DEFAULT_CHARACTER_NAME;
	static const uint32 DEFAULT_CHARACTER_NAME_MIN_ID;
	static const uint32 DEFAULT_CHARACTER_NAME_MAX_ID;


#pragma endregion

public:
	MurphysLawGameSettings();
	~MurphysLawGameSettings();

#pragma region Members
public:
	// Time of the game
	int32 GameTime;
	// Is a warmup wanted
	bool WarmupWanted;
	// The number of points a team needs to have to win
	int32 NbPointsForWin;
	// The number of teams involved in the game
	int32 NbTeams;
	// The number of players per team
	int32 NbPlayersPerTeam;
	// The name of the player
	FString CharacterName;
	// The game name
	FString GameName;
#pragma endregion

#pragma region Encoding and decoding functions
public:
	// Marshalling function that includes leading separator
	FString Serialize() const;

	// Unmarshalling function
	static MurphysLawGameSettings Parse(const FString& DataUrl);
#pragma endregion
};

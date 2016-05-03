// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/**
 * 
 */
class MURPHYSLAW_API MurphysLawTeamColor
{
	static const float DEFAULT_SATURATION_FACTOR;
	static const MurphysLawTeamColor PREDEFINED_TEAM_COLORS[];

	FColor TeamColorPrimary;
	FColor TeamColorDarker;

public:
	MurphysLawTeamColor(const FString& TeamColor);
	MurphysLawTeamColor(const FString& PrimaryTeamColor, const FString& AlternateTeamColor);
	MurphysLawTeamColor(const FColor& TeamColor);
	MurphysLawTeamColor(const FColor& PrimaryTeamColor, const FColor& AlternateTeamColor);

	~MurphysLawTeamColor();

	bool operator==(const MurphysLawTeamColor& other) const;

	const FColor GetPrimary() const;
	const FColor GetDarker() const;

	static TArray<MurphysLawTeamColor> GetPredefinedColors(const int32 NbColors);
};

// Fill out your copyright notice in the Description page of Project Settings.

#include "MurphysLaw.h"
#include "MurphysLawTeamColor.h"
#include <MurphysLaw/Utils/MurphysLawUtils.h>

// Constants
const float MurphysLawTeamColor::DEFAULT_SATURATION_FACTOR(0.3f);
const MurphysLawTeamColor MurphysLawTeamColor::PREDEFINED_TEAM_COLORS[] = {
	// Primaries
	MurphysLawTeamColor("#FF0000"),
	MurphysLawTeamColor("#5582e2"),
	MurphysLawTeamColor("#FFFF00"),
	MurphysLawTeamColor("#00FFFF"),
	// Darker
	MurphysLawTeamColor("#00E600"),
	// Primary combinations
	MurphysLawTeamColor("#FF8000"),
	MurphysLawTeamColor("#8000FF"),
	// Black and white
	MurphysLawTeamColor("#000000"),
	MurphysLawTeamColor("#FFFFFF")
};


MurphysLawTeamColor::MurphysLawTeamColor(const FString& TeamColor)
	:	TeamColorPrimary{ FColor::FromHex(TeamColor) },
		TeamColorDarker{ FColor(TeamColorPrimary.R * DEFAULT_SATURATION_FACTOR,
								TeamColorPrimary.G * DEFAULT_SATURATION_FACTOR,
								TeamColorPrimary.B * DEFAULT_SATURATION_FACTOR) }
{}

MurphysLawTeamColor::MurphysLawTeamColor(const FString& PrimaryTeamColor, const FString& AlternateTeamColor)
	:	TeamColorPrimary{ FColor::FromHex(PrimaryTeamColor) },
		TeamColorDarker{ FColor::FromHex(AlternateTeamColor) }
{}

MurphysLawTeamColor::MurphysLawTeamColor(const FColor& TeamColor)
	:	TeamColorPrimary { TeamColor },
		TeamColorDarker{ FColor(TeamColorPrimary.R * DEFAULT_SATURATION_FACTOR,
			TeamColorPrimary.G * DEFAULT_SATURATION_FACTOR,
			TeamColorPrimary.B * DEFAULT_SATURATION_FACTOR) }
{}

MurphysLawTeamColor::MurphysLawTeamColor(const FColor& PrimaryTeamColor, const FColor& AlternateTeamColor)
	:	TeamColorPrimary{ PrimaryTeamColor },
		TeamColorDarker{ AlternateTeamColor }
{}

MurphysLawTeamColor::~MurphysLawTeamColor()
{
}

const FColor MurphysLawTeamColor::GetPrimary() const { return TeamColorPrimary; }
const FColor MurphysLawTeamColor::GetDarker() const { return TeamColorDarker; }


bool MurphysLawTeamColor::operator==(const MurphysLawTeamColor& other) const
{
	return TeamColorPrimary == other.TeamColorPrimary && TeamColorDarker == TeamColorDarker;
}

TArray<MurphysLawTeamColor> MurphysLawTeamColor::GetPredefinedColors(const int32 NbColors)
{
	static const int32 ColorArrayLength = MurphysLawUtils::ArrayLength(PREDEFINED_TEAM_COLORS);
	checkf(NbColors <= ColorArrayLength, TEXT("Number of requested color is to high"));

	TArray<MurphysLawTeamColor> TeamColors;
	while (TeamColors.Num() != NbColors)
	{
		const MurphysLawTeamColor SelectedColor = PREDEFINED_TEAM_COLORS[FMath::RandRange(0, ColorArrayLength - 1)];
		if(!TeamColors.Contains(SelectedColor)) TeamColors.Add(SelectedColor);
	}

	return TeamColors;
}

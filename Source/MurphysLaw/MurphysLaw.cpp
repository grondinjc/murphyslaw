// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "MurphysLaw.h"


IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, MurphysLaw, "MurphysLaw");

void ShowInfo(const char* c, const float DisplayTime) { ShowInfo(FString(c), DisplayTime); }
void ShowInfo(const FString& s, const float DisplayTime)
{
	ShowMessage(s, FColor::Green, DisplayTime);
}

void ShowWarning(const char* c, const float DisplayTime) { ShowWarning(FString(c), DisplayTime); }
void ShowWarning(const FString& s, const float DisplayTime)
{
	ShowMessage(s, FColor::Yellow, DisplayTime);
}

void ShowError(const char* c, const float DisplayTime) { ShowError(FString(c), DisplayTime); }
void ShowError(const FString& s, const float DisplayTime)
{
	ShowMessage(s, FColor::Red, DisplayTime);
}

void ShowMessage(const FString& s, const FColor& Color, const float DisplayTime, int32 key)
{
	//const int32 Key = FMath::Rand();
	UE_LOG(LogTemp, Warning, TEXT("%s"), *s);
	//GEngine->AddOnScreenDebugMessage(key, DisplayTime, Color, s);
}

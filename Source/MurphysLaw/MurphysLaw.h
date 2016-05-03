// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#ifndef __MURPHYSLAW_H__
#define __MURPHYSLAW_H__

#include "Engine.h"
#include "UnrealNetwork.h"
#include "Online.h"

void ShowInfo(const char* c, const float DisplayTime = 5.f);
void ShowInfo(const FString& s, const float DisplayTime = 5.f);

void ShowWarning(const char* c, const float DisplayTime = 5.f);
void ShowWarning(const FString& s, const float DisplayTime = 5.f);

void ShowError(const char* c, const float DisplayTime = 5.f);
void ShowError(const FString& s, const float DisplayTime = 5.f);

void ShowMessage(const FString& s, const FColor& Color = FColor::White, const float DisplayTime = 5.f, int32 key = -1);

#endif

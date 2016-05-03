// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "MurphysLawIController.generated.h"

/**
Interface representing the commun action for AI and human controller
*/
UINTERFACE(Blueprintable)
class UMurphysLawIController : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class MURPHYSLAW_API IMurphysLawIController
{
	GENERATED_IINTERFACE_BODY()

public:

	// Event when controlled pawn has died
	virtual void OnKilled(const float TimeToRespawn) = 0;

	// Logic to respawn character
	virtual void Respawn() = 0;
};

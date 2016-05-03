// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerStart.h"
#include "MurphysLawPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class MURPHYSLAW_API AMurphysLawPlayerStart : public APlayerStart
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Team")
	int32 Team;
	
	
};

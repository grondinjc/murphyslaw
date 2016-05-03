// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "MurphysLawSkySphereBase.generated.h"

UCLASS(abstract)
class MURPHYSLAW_API AMurphysLawSkySphereBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMurphysLawSkySphereBase();

	// Update sky's look based on sun height
	UFUNCTION(BlueprintNativeEvent)
	void UpdateSunOrientation();
	virtual void UpdateSunOrientation_Implementation() { }
};

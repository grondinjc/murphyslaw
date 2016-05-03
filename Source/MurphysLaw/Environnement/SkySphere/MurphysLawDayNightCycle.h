// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "MurphysLawDayNightCycle.generated.h"

/**
This class is an addon to the sky sphere.
It is responsible for rotating the sun (directional light)

It auto detects required scene elements. Therefore, it just
needs to be dragged in the scene.
*/
UCLASS()
class MURPHYSLAW_API AMurphysLawDayNightCycle : public AActor
{
	GENERATED_BODY()

	static const float NB_SECONDS_IN_REAL_MINUTES;
	static const float NB_HOURS_IN_REAL_DAY;

	/** Factor converting a hour-time value ([0; 24[) to a degree value ([0; 360[)*/
	static const float HOUR_TO_DEGREE_FACTOR;
	/** Angle offset to position a 6 o'clock sun on the "left" rather than "down" on the rotation axis */
	static const float SUN_DEGREE_OFFSET;

public:	
	// Sets default values for this actor's properties
	AMurphysLawDayNightCycle();

	/** Defines replication with members */
	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

protected:
	/** The number of minutes for the sun to do a full cycle */
	UPROPERTY(EditDefaultsOnly, Category = "In Game time")
	float DayLengthInMinutes;

	/** [0; 24[ The initial hour of the sun position where 12 is mid-day and 0 is noon */
	UPROPERTY(EditDefaultsOnly, Category = "In Game time")
	float InitialInGameHour;

private:
	/** A reference to the ambiant light */
	class ASkyLight* SkyLight;

	/** A reference to the directional light to update its position */
	class ADirectionalLight* SunLight;

	/** A reference to the sky dome to update sky based on sun height in the sky */
	class AMurphysLawSkySphereBase* SkyDome;

	/** The angle of the sun rotation */
	UPROPERTY(VisibleAnywhere, Category = "In Game time")
	float SunAngle;

	/** The current virtual hour in the game */
	UPROPERTY(VisibleAnywhere, Replicated, Category = "In Game time")
	float CurrentInGameHour;

	/** The duration in seconds of a complete virtual day of 24 hours */
	float InGameHourDuration;

	/** Set scene parameters */
	void InitializeSceneReference();
};

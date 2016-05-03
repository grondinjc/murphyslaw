// Fill out your copyright notice in the Description page of Project Settings.

#include "../../MurphysLaw.h"
#include "MurphysLawDayNightCycle.h"
#include "MurphysLawSkySphereBase.h"
#include "../../Utils/MurphysLawUtils.h"

#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "UnrealNetwork.h"

// Constants
const float AMurphysLawDayNightCycle::NB_SECONDS_IN_REAL_MINUTES(60.f);
const float AMurphysLawDayNightCycle::NB_HOURS_IN_REAL_DAY(24.f);
const float AMurphysLawDayNightCycle::HOUR_TO_DEGREE_FACTOR(360.f / NB_HOURS_IN_REAL_DAY);
const float AMurphysLawDayNightCycle::SUN_DEGREE_OFFSET(90.f);


// Sets default values
AMurphysLawDayNightCycle::AMurphysLawDayNightCycle()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bReplicates = true;		// Enable network replication
	bCanBeDamaged = false;	// Disable collision and damage system
	bBlockInput = true;		// Disable keyboard input
	
	/** Since the "client" cannot communicate to the server, the server will replicate the current in-game time.
	However, since this is network intensive, the replication frequency will be very low */
	NetUpdateFrequency = MIN_flt; // Number of replication per seconds

	// Defaults time properties
	DayLengthInMinutes = 2.f;
	InitialInGameHour = float(FMath::RandRange(0, 23));

	// Actor references initialisation
	SunLight = nullptr;
	SkyDome = nullptr;
	SkyLight = nullptr;
}

/** Defines replication with members */
void AMurphysLawDayNightCycle::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMurphysLawDayNightCycle, CurrentInGameHour);
}

// Called when the game starts or when spawned
void AMurphysLawDayNightCycle::BeginPlay()
{
	Super::BeginPlay();

	// Evaluate accelerated time metrics
	float OneInGameDayTimeInSeconds = DayLengthInMinutes * NB_SECONDS_IN_REAL_MINUTES;
	InGameHourDuration = OneInGameDayTimeInSeconds / NB_HOURS_IN_REAL_DAY;

	CurrentInGameHour = InitialInGameHour;

	// Retreive scene reference
	SkyLight = MurphysLawUtils::GetUniqueSceneReference<ASkyLight>(this);
	SunLight = MurphysLawUtils::GetUniqueSceneReference<ADirectionalLight>(this);
	SkyDome = MurphysLawUtils::GetUniqueSceneReference<AMurphysLawSkySphereBase>(this);

	// Turn off Tick if actor were not detected in the scene
	if(SunLight != nullptr && SkyDome != nullptr && SkyLight != nullptr)
	{
		InitializeSceneReference();
		SetActorTickEnabled(true);
	}
	else
	{
		ShowWarning("Missing scene actors for day and night cycle");
		SetActorTickEnabled(false);
	}
}

// Scene reference initialization
void AMurphysLawDayNightCycle::InitializeSceneReference()
{
	SunLight->SetMobility(EComponentMobility::Movable);
}

// Called every frame
void AMurphysLawDayNightCycle::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	// Update position angle
	CurrentInGameHour += (DeltaTime / InGameHourDuration);
	if (CurrentInGameHour >= NB_HOURS_IN_REAL_DAY) CurrentInGameHour -= NB_HOURS_IN_REAL_DAY;

	SunAngle = (CurrentInGameHour * HOUR_TO_DEGREE_FACTOR) + SUN_DEGREE_OFFSET;
	if(SunAngle >= 360.f) SunAngle -= 360.f;

	// Refresh sky's display
	SunLight->SetActorRotation(FRotator(SunAngle, 0.f, 0.f));
	SkyDome->UpdateSunOrientation();
}



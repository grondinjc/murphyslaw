// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MurphysLawIObjectCollector.generated.h"

/** 
Interface representing someone that can collect object such as a gun, health pickups, ammo, etc.*/
UINTERFACE(Blueprintable)
class UMurphysLawIObjectCollector : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class MURPHYSLAW_API IMurphysLawIObjectCollector
{
	GENERATED_IINTERFACE_BODY()

public:

	/** Receive an amount of healing from something (environment, pickup ...) */
	virtual void ReceiveHealAmount(const float Heal) = 0;

	/** Receive ammos from something (environment, pickup ...) */
	virtual void ReceiveAmmo(const int32 NumberOfAmmo) = 0;

	/** Receive ammos from something (environment, pickup ...) */
	virtual void CollectWeapon(class AMurphysLawBaseWeapon* Weapon) = 0;
};
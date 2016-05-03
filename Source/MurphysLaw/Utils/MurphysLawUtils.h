// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineUtils.h"

/*
 * An helper class to interact with the scene. 
 */
class MURPHYSLAW_API MurphysLawUtils
{
public:
	/** Print to log and screen the actor role */
	static void ShowRemoteRole(const AActor* NetworkActor, int32 KeyPrint = -1);

	// Team verifications
	static bool IsInSameTeam(const class AActor* const  PlayerA, const class AActor* const  PlayerB);
	static bool IsInSameTeam(const class ACharacter* const PlayerA, const class ACharacter* const  PlayerB);
	static bool IsInSameTeam(const class AController* const PlayerA, const class AController* const  PlayerB);
	static bool IsInSameTeam(const class APlayerState* const PlayerA, const class APlayerState* const PlayerB);
	static bool IsInSameTeam(const class AMurphysLawPlayerState* const PlayerA, const class AMurphysLawPlayerState* const PlayerB);

	// Respawn characters
	static void RespawnCharacter(class AMurphysLawGameMode* GameMode, class AMurphysLawPlayerState* MurphysLawPlayerState, class AMurphysLawCharacter* MyCharacter);

	// Primitive convertions
	static FString IntToString(const int32 Value);
	static FString BoolToString(const bool Value);
	static FString FloatToString(const float Value);
	static int32 StringToInt(const FString& Value);
	static bool StringToBool(const FString& Value);
	static float StringToFloat(const FString& Value);


	/*	Gives the size of an array of form 
		T MyArray[] = { ... };
	*/ 
	template<class T, int32 N>
	static int32 ArrayLength(T(&)[N]) { return N; }

	/**
	Retreive a reference to an actor in the scene.
	@param InterrogatingActor An actor required to create the iterator.
	@return The reference of the found actor or null if there are zero or at least two object found.
	*/
	template<class T>
	static T* GetUniqueSceneReference(const AActor* InterrogatingActor)
	{
		T* SceneObject = nullptr;

		TArray<T*> FoundObjects;
		for (TActorIterator<T> It(InterrogatingActor->GetWorld()); It; ++It) FoundObjects.Add(*It);

		switch (FoundObjects.Num())
		{
			case 1:
				SceneObject = FoundObjects[0];
				break;

			case 0:
				ShowWarning("No reference found for @@@");
				break;

			default:
				ShowWarning("Too much references found for @@@");
		}

		return SceneObject;
	}

	/**
	Retreive all references to an actor in the scene.
	@param InterrogatingActor An actor required to create the iterator.
	@return The references of the found actors.
	*/
	template<class T>
	static TArray<T*> GetAllSceneReferences(const AActor* InterrogatingActor)
	{
		TArray<T*> FoundObjects;
		for (TActorIterator<T> It(InterrogatingActor->GetWorld()); It; ++It) FoundObjects.Add(*It);

		return FoundObjects;
	}
};

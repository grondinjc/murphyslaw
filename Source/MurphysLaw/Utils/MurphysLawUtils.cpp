// Fill out your copyright notice in the Description page of Project Settings.

#include "MurphysLaw.h"
#include "MurphysLawUtils.h"
#include "../Network/MurphysLawPlayerState.h"

#include "Kismet/KismetStringLibrary.h"
#include "Serialization/Archive.h"
#include "Misc/CoreMisc.h"

// Team verifications
bool MurphysLawUtils::IsInSameTeam(const AActor* const PlayerA, const AActor* const  PlayerB)
{
	return IsInSameTeam(Cast<ACharacter>(PlayerA), Cast<ACharacter>(PlayerB));
}
bool MurphysLawUtils::IsInSameTeam(const ACharacter* const PlayerA, const ACharacter* const  PlayerB)
{
	return PlayerA != nullptr && PlayerB != nullptr &&
		IsInSameTeam(PlayerA->GetController(), PlayerB->GetController());
}
bool MurphysLawUtils::IsInSameTeam(const class AController* const PlayerA, const class AController* const  PlayerB)
{
	return PlayerA != nullptr && PlayerB != nullptr &&
		IsInSameTeam(PlayerA->PlayerState, PlayerB->PlayerState);
}
bool MurphysLawUtils::IsInSameTeam(const APlayerState* const PlayerA, const APlayerState* const  PlayerB)
{
	return IsInSameTeam(Cast<AMurphysLawPlayerState>(PlayerA), Cast<AMurphysLawPlayerState>(PlayerB));
}
bool MurphysLawUtils::IsInSameTeam(const AMurphysLawPlayerState* const PlayerA, const AMurphysLawPlayerState* const  PlayerB)
{
	return PlayerA != nullptr && PlayerB != nullptr && PlayerA->GetTeam() == PlayerB->GetTeam();
}

// Primitive convertions
FString MurphysLawUtils::IntToString(const int32 Value) { return UKismetStringLibrary::Conv_IntToString(Value); }
FString MurphysLawUtils::BoolToString(const bool Value) { return UKismetStringLibrary::Conv_BoolToString(Value); }
FString MurphysLawUtils::FloatToString(const float Value) { return UKismetStringLibrary::Conv_FloatToString(Value); }
int32 MurphysLawUtils::StringToInt(const FString& Value) { return UKismetStringLibrary::Conv_StringToInt(Value); }
bool MurphysLawUtils::StringToBool(const FString& Value) { return Value.ToBool(); }
float MurphysLawUtils::StringToFloat(const FString& Value) { return UKismetStringLibrary::Conv_StringToFloat(Value); }

// Debug remote role
void MurphysLawUtils::ShowRemoteRole(const AActor* NetworkActor, int32 KeyPrint)
{
	const auto RoleString = (NetworkActor->Role == ROLE_Authority) ? "ROLE_Authority" :
						((NetworkActor->Role == ROLE_AutonomousProxy) ? "ROLE_AutonomousProxy" :
						((NetworkActor->Role == ROLE_SimulatedProxy) ? "ROLE_SimulatedProxy" :
						((NetworkActor->Role == ROLE_None) ? "ROLE_None" : "ROLE_???")));
	ShowWarning(FString::Printf(TEXT("%s is %s"), *NetworkActor->GetHumanReadableName(), *FString(RoleString)), KeyPrint);
}

void MurphysLawUtils::RespawnCharacter(AMurphysLawGameMode* GameMode, AMurphysLawPlayerState* MurphysLawPlayerState, AMurphysLawCharacter* MyCharacter)
{
	// Exclude controllers without characters
	if(MyCharacter != nullptr)
	{
		AActor* PlayerStart = GameMode->ChoosePlayerStart(MurphysLawPlayerState->GetTeam());
		MyCharacter->Relive();
		MyCharacter->TeleportTo(PlayerStart->GetActorLocation(), PlayerStart->GetActorRotation());
	}
}

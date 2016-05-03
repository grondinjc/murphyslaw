// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MurphysLawExplosiveBarrel.generated.h"

/**
*
*/
UCLASS()
class MURPHYSLAW_API AMurphysLawExplosiveBarrel : public AActor
{
	GENERATED_BODY()

#pragma region Destruction members
private:
	/** The component that will handle destructions */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
	class UDestructibleComponent* DestructibleObject;

	/** Represents if inflicted damage is the same for within explosion radius */
	UPROPERTY(EditDefaultsOnly, Category = "Destruction")
	bool ExplosionDoFullDamage;

	/** Represents the force at which fragments will be expelled */
	UPROPERTY(EditDefaultsOnly, Category = "Destruction")
	float ExplosionImpulseStrength;

	/** Represents the outter radius of the explosion; the biggest radius of a donut-shape */
	UPROPERTY(EditDefaultsOnly, Category = "Destruction")
	float ExplosionOutterRadius;

	/** Represents the outter radius of the explosion; the smallest radius of a donut-shape  */
	UPROPERTY(EditDefaultsOnly, Category = "Destruction")
	float ExplosionInnerRadius;

	/** Represents the damage caused uppon explosion */
	UPROPERTY(EditDefaultsOnly, Category = "Destruction")
	float ExplosionDamage;

	/** Represents the minimal damage caused by an explosion */
	UPROPERTY(EditDefaultsOnly, Category = "Destruction")
	float ExplosionMinimalDamage;

#pragma endregion

#pragma region Health members
private:

	/** The maximum health level of the character */
	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float MaxHealth;

	/** The current health level of the character */
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Health")
	float CurrentHealth;

#pragma endregion

public:
	AMurphysLawExplosiveBarrel();
	
	/** Defines replication with members */
	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	/** Called when the game starts or when spawned */
	void BeginPlay() override;

	/** Entry point for received damages */
	float TakeDamage(float DamageAmount, const FDamageEvent & DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_TakeDamage(float DamageAmount, const FDamageEvent & DamageEvent, AController* EventInstigator, AActor* DamageCauser);

	/** Called from blueprint; Server-side notification that any type of damage were received */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Damage")
	void OnReceiveAnyDamage(float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);
	
	/** Allows the server to destroy the barrel and execute the same routine on all clients */
	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void Multicast_Explode();
};

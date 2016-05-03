// Fill out your copyright notice in the Description page of Project Settings.

#include "MurphysLaw.h"
#include "MurphysLawExplosiveBarrel.h"

#include "UnrealNetwork.h"


AMurphysLawExplosiveBarrel::AMurphysLawExplosiveBarrel()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Enable network replication
	bReplicates = true;

	// Health properties
	MaxHealth = 10.f;	

	// Set member default values
	ExplosionImpulseStrength = 15000.f;
	ExplosionOutterRadius = 2500.f;		// 25 meters
	ExplosionInnerRadius = MIN_flt;		// 0 meters
	ExplosionDoFullDamage = true;
	ExplosionDamage = 200.f;
	ExplosionMinimalDamage = 10.f;

	// Create the destructible component
	DestructibleObject = CreateDefaultSubobject<UDestructibleComponent>(TEXT("DestructibleComponent"));
	DestructibleObject->AttachTo(RootComponent);
}

void AMurphysLawExplosiveBarrel::BeginPlay() 
{
	CurrentHealth = MaxHealth;

	// Check for missing or invalid resources
	if(DestructibleObject->GetDestructibleMesh() == nullptr) ShowWarning("ExplosiveBarrel - No destructible mesh assigned");
}

// Defines replicated members
void AMurphysLawExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMurphysLawExplosiveBarrel, CurrentHealth);
}

// Entry point for any type of damage
float AMurphysLawExplosiveBarrel::TakeDamage(float DamageAmount, const FDamageEvent & DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = 0.f;

	if (CurrentHealth > 0)
	{
		if (Role == ROLE_Authority)
		{
			// Intercept damage event to avoid communication to server when the barrel has exploded
			ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
		}
		else
		{
			// Let the server handle it
			Server_TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
		}
	}

	return ActualDamage;
}

bool AMurphysLawExplosiveBarrel::Server_TakeDamage_Validate(float DamageAmount, const FDamageEvent & DamageEvent, AController* EventInstigator, AActor* DamageCauser) { return true; }
void AMurphysLawExplosiveBarrel::Server_TakeDamage_Implementation(float DamageAmount, const FDamageEvent & DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}


// Executed by server and then by clients
bool AMurphysLawExplosiveBarrel::Multicast_Explode_Validate() { return true; }
void AMurphysLawExplosiveBarrel::Multicast_Explode_Implementation()
{
	// Destroy the destructable only !!!
	DestructibleObject->ApplyRadiusDamage(MAX_FLT, GetActorLocation(), ExplosionOutterRadius, ExplosionImpulseStrength, ExplosionDoFullDamage);
}

// Server-side notification that damages were received
void AMurphysLawExplosiveBarrel::OnReceiveAnyDamage(float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	// Check if not dead yet and that inflicted damage just killed it
	if (CurrentHealth > 0)
	{
		CurrentHealth = FMath::Max(CurrentHealth - Damage, 0.f);

		if (CurrentHealth == 0)
		{
			// Destroy the barrel
			Multicast_Explode();

			// Deal damage over zone
			TArray<AActor*> IgnoredActors;
			UGameplayStatics::ApplyRadialDamageWithFalloff(this, ExplosionDamage, ExplosionMinimalDamage, GetActorLocation(), ExplosionInnerRadius, ExplosionOutterRadius, 10.f, UDamageType::StaticClass(), IgnoredActors, this, InstigatedBy);
		}
	}
}

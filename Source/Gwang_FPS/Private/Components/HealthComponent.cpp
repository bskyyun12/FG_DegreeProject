// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/HealthComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	Reset();
}

float UHealthComponent::GetHealth()
{
	return CurrentHealth;
}

float UHealthComponent::GetArmor()
{
	return CurrentArmor;
}

// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	SetIsReplicated(true);
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHealthComponent, CurrentHealth);
	DOREPLIFETIME(UHealthComponent, CurrentArmor);
}

void UHealthComponent::Reset()
{
	CurrentHealth = MaxHealth;	// OnRep_CurrentHealth
	CurrentArmor = MaxArmor;	// OnRep_CurrentArmor
}

void UHealthComponent::Server_TakeDamage_Implementation(AActor* DamageSource, float Damage)
{
	CurrentHealth -= Damage;	// OnRep_CurrentHealth
	OnTakeDamage.Broadcast(DamageSource);
	OnUpdateHealthArmorUI.Broadcast();

	if (CurrentHealth <= 0.f)
	{
		OnDeath.Broadcast(DamageSource);
	}
}

void UHealthComponent::Server_AcquireHealth_Implementation(AActor* HealthSource, float Value)
{
	CurrentHealth += Value;	// OnRep_CurrentHealth

	OnHealthAcquired.Broadcast(HealthSource);
}

bool UHealthComponent::IsDead()
{
	return CurrentHealth <= 0.f;
}

void UHealthComponent::OnRep_CurrentHealth()
{
	OnUpdateHealthArmorUI.Broadcast();
}

void UHealthComponent::OnRep_CurrentArmor()
{
	OnUpdateHealthArmorUI.Broadcast();
}
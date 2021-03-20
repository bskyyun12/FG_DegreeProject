// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/HealthComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	SetIsReplicated(true);
	Reset();
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHealthComponent, bIsDead);
}

void UHealthComponent::Reset()
{
	bIsDead = false; // OnRep_bIsDead
	CurrentHealth = MaxHealth;
}

void UHealthComponent::Server_AddHealth_Implementation(float ValueToAdd)
{
	CurrentHealth += ValueToAdd;

	if (ValueToAdd > 0.f)
	{
		OnHealthAcquired.Broadcast();
	}

	if (ValueToAdd < 0.f)
	{
		OnDamageReceived.Broadcast();
	}

	if (CurrentHealth <= 0.f)
	{
		bIsDead = true;	// OnRep_bIsDead
		OnDeath.Broadcast();
	}
}

bool UHealthComponent::IsDead()
{
	return bIsDead;
}

void UHealthComponent::OnRep_bIsDead()
{
	if (bIsDead)
	{
		OnDeath.Broadcast();
	}
}

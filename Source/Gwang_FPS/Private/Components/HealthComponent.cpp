// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/HealthComponent.h"

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

	Reset();
}

void UHealthComponent::Reset()
{
	CurrentHealth = MaxHealth;
}

void UHealthComponent::AddHealth(float ValueToAdd)
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
		OnDeath.Broadcast();
	}
}

bool UHealthComponent::IsDead()
{
	return CurrentHealth <= 0.f;
}

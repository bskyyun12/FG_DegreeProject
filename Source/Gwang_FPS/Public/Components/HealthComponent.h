// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GWANG_FPS_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

	void AddHealth(float ValueToAdd);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;		

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHealthDelegate);
	
	UPROPERTY(BlueprintAssignable)
	FHealthDelegate OnDamageReceived;

	UPROPERTY(BlueprintAssignable)
	FHealthDelegate OnHealthAcquired;

	UPROPERTY(BlueprintAssignable)
	FHealthDelegate OnDeath;

private:
	UPROPERTY(EditDefaultsOnly)
	float MaxHealth = 100.f;

	float CurrentHealth;
};

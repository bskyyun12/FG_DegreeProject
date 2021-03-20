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

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void Reset();

	UFUNCTION(Server, Reliable)
	void Server_AddHealth(float ValueToAdd);

	bool IsDead();

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHealthDelegate);
	
	UPROPERTY(BlueprintAssignable)
	FHealthDelegate OnDamageReceived;

	UPROPERTY(BlueprintAssignable)
	FHealthDelegate OnHealthAcquired;

	UPROPERTY(BlueprintAssignable)
	FHealthDelegate OnDeath;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;		

private:
	UPROPERTY(EditDefaultsOnly)
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing=OnRep_bIsDead)
	bool bIsDead;

	UFUNCTION()
	void OnRep_bIsDead();

	float CurrentHealth;
};

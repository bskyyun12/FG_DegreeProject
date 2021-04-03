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

	float GetHealth();

	float GetArmor();

	UFUNCTION(Server, Reliable)
	void Server_OnSpawn();

	UFUNCTION(Server, Reliable)		
	void Server_TakeDamage(AActor* DamageSource, float DamageOnHealth, float DamageOnArmor);

	UFUNCTION(Server, Reliable)
	void Server_AcquireHealth(AActor* HealthSource, float Value);

	bool IsDead();

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FResetDelegate);
	FResetDelegate OnSpawn;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHealthDelegate, AActor*, Source);
	FHealthDelegate OnTakeDamage;
	FHealthDelegate OnHealthAcquired;
	FHealthDelegate OnDeath;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHealthArmorUIDelegate);
	FHealthArmorUIDelegate OnUpdateHealthArmorUI;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(EditDefaultsOnly)
	float MaxHealth = 100.f;

	UPROPERTY(EditDefaultsOnly)
	float MaxArmor = 100.f;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentHealth)
	float CurrentHealth = 100.f;
	UFUNCTION()
	void OnRep_CurrentHealth();

	UPROPERTY(ReplicatedUsing = OnRep_CurrentArmor)
	float CurrentArmor = 100.f;
	UFUNCTION()
	void OnRep_CurrentArmor();

	UPROPERTY(ReplicatedUsing = OnRep_bIsDead)
	bool bIsDead = false;
	UFUNCTION()
	void OnRep_bIsDead();
};

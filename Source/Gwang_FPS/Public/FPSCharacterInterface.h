// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FPSCharacterInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UFPSCharacterInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GWANG_FPS_API IFPSCharacterInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	// Getters
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FTransform GetCameraTransform();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	USkeletalMeshComponent* GetCharacterMesh();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	USkeletalMeshComponent* GetArmMesh();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	float GetHealth();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	float GetArmor();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void TakeDamage(AActor* DamageCauser, float DamageOnHealth, float DamageOnArmor, FVector const& HitPoint);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnSpawnPlayer();
};

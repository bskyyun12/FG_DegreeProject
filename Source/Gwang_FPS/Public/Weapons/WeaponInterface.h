// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WeaponInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UWeaponInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GWANG_FPS_API IWeaponInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	EWeaponType GetWeaponType() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnWeaponEquipped(ADeathMatchCharacter* NewOwner);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnWeaponDropped();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void BeginFire();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void EndFire();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Reload();
};

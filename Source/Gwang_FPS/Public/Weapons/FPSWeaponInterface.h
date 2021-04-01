// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FPSWeaponInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UFPSWeaponInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GWANG_FPS_API IFPSWeaponInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	AFPSWeaponBase* GetWeapon();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ToggleVisibility(bool bNewVisibility);
};

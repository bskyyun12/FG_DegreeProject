// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FPSAnimInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UFPSAnimInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GWANG_FPS_API IFPSAnimInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void UpdateSpineAngle(float Pitch);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void HandleCrouch(bool bCrouchButtonDown);

};

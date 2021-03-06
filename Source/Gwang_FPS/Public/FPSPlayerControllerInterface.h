// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FPSPlayerControllerInterface.generated.h"

class AFPSCharacter;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UFPSPlayerControllerInterface : public UInterface
{
	GENERATED_BODY()
};

class GWANG_FPS_API IFPSPlayerControllerInterface
{
	GENERATED_BODY()

		// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void LoadTeamSelection();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnDarkTeamSelected();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnSilverTeamSelected();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnSpawnPlayer(TSubclassOf<AFPSCharacter> CharacterClass, FTransform Transform);
};

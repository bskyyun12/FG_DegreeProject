// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FPSGameMode.h"
#include "FPSPlayerControllerInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UFPSPlayerControllerInterface : public UInterface
{
	GENERATED_BODY()
};

class GWANG_FPS_API IFPSPlayerControllerInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void StartNewGame();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnTeamSelected(ETeam InTeam);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnSpawnPlayer(AFPSCharacter* Player);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void RespawnPlayer();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnPlayerDeath();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void LoadGameOver(bool Victory);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ShakeCamera(TSubclassOf<UCameraShakeBase> CameraShake);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void AddControlRotation(const FRotator& RotationToAdd);
};

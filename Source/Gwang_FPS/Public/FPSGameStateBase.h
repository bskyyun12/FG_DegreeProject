// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "FPSPlayerStart.h"
#include "FPSGameStateBase.generated.h"

class AFPSGameMode;

UCLASS()
class GWANG_FPS_API AFPSGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	void Initialize(AFPSGameMode* GameMode);

	void OnPlayerDeath(ETeam Team);

private:
	UPROPERTY()
	AFPSGameMode* FPSGameMode;

	UPROPERTY(Replicated)
	int MarvelScore;

	UPROPERTY(Replicated)
	int DCScore;

private:
	UFUNCTION()
	void OnStartNewGame();
};

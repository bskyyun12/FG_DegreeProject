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
	UPROPERTY(ReplicatedUsing = OnRep_MarvelScore)
	int MarvelScore;

	UFUNCTION()
	void OnRep_MarvelScore();

	UPROPERTY(ReplicatedUsing = OnRep_DCScore)
	int DCScore;

	UFUNCTION()
	void OnRep_DCScore();

	UPROPERTY()
	AFPSGameMode* FPSGameMode;

private:
	UFUNCTION()
	void OnStartNewGame();
};

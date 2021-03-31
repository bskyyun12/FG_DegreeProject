// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "FPSGameInstance.h"
#include "FPSGameState.generated.h"

class AFPSGameMode;

UCLASS()
class GWANG_FPS_API AFPSGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	void Init(AFPSGameMode* GameMode);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FScoreUpdateDelegate, int, MarvelTeamScore, int, DCTeamScore);
	FScoreUpdateDelegate OnUpdateScore;

protected:
	UPROPERTY()
	AFPSGameMode* FPSGameMode;

	UPROPERTY(ReplicatedUsing=OnRep_MarvelScore)
	int MarvelScore;
	UFUNCTION()
	void OnRep_MarvelScore();

	UPROPERTY(ReplicatedUsing= OnRep_DCScore)
	int DCScore;
	UFUNCTION()
	void OnRep_DCScore();

protected:
	UFUNCTION()
	void OnStartMatch();

	UFUNCTION()
	void OnEndMatch();
};

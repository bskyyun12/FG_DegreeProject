// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "FPSGameInstance.h"
#include "DeathMatchGameState.generated.h"

class ADeathMatchGameMode;
class ADeathMatchPlayerState;

UCLASS()
class GWANG_FPS_API ADeathMatchGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	void AddScore(const ETeam& WinnerTeam);

protected:
	ADeathMatchGameMode* GM;

	UPROPERTY(Replicated)
	uint8 MarvelTeamScore;

	UPROPERTY(Replicated)
	uint8 DCTeamScore;

	UPROPERTY(Replicated)
	TArray<ADeathMatchPlayerState*> MarvelPlayerStates;

	UPROPERTY(Replicated)
	TArray<ADeathMatchPlayerState*> DCPlayerStates;

	UPROPERTY(Replicated)
	float TimeLeftInSeconds;

	FTimerHandle MatchTimer;

protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void PostInitializeComponents() override;

	void BeginPlay() override;

	UFUNCTION()
	void MatchTimeCountdown();	
};

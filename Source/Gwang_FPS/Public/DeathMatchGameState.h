// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "FPSGameInstance.h"
#include "DeathMatchGameState.generated.h"

class ADeathMatchGameMode;
class ADeathMatchPlayerState;

USTRUCT(BlueprintType)
struct FScoreboardData
{
	GENERATED_BODY()

	UPROPERTY()
	int32 PlayerId;

	UPROPERTY()
	FName PlayerName;

	UPROPERTY()
	ETeam Team;

	UPROPERTY()
	uint8 Kills;

	UPROPERTY()
	uint8 Deaths;

	FScoreboardData()
	{
		PlayerId = -1;
		PlayerName = "Gwang";
		Team = ETeam::None;
		Kills = -1;
		Deaths = -1;
	}

	FScoreboardData(const int32& Id, const FName& Name, const ETeam& _Team)
	{
		PlayerId = Id;
		PlayerName = Name;
		Team = _Team;
		Kills = 0;
		Deaths = 0;
	}

	void operator = (const FScoreboardData& NewData) 
	{
		PlayerId = NewData.PlayerId;
		PlayerName = NewData.PlayerName;
		Team = NewData.Team;
		Kills = NewData.Kills;
		Deaths = NewData.Deaths;
	}
};

UCLASS()
class GWANG_FPS_API ADeathMatchGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	// Scoreboard
	FScoreboardData GetScoreboardData(ADeathMatchPlayerState* PS);
	void SetScoreboardData(const FScoreboardData& Data);

	// Team score
	void AddScore(const ETeam& TeamToAddScore);

protected:
	UPROPERTY(Replicated)
	uint8 MarvelTeamScore;

	UPROPERTY(Replicated)
	uint8 DCTeamScore;

	ADeathMatchGameMode* GM;

	TArray<FScoreboardData> ScoreboardData;

	int TimeLeftInSeconds;

	int ScoreToWin;

	FTimerHandle MatchTimer;

protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void BeginPlay() override;

	UFUNCTION()
	void MatchTimeCountdown();

	void UpdateScoreBoard();
	
	void EndMatch();
};

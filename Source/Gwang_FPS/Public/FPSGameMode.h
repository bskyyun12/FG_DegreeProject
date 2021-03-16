// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "FPSPlayerStart.h"

#include "FPSGameMode.generated.h"

class AFPSCharacter;

UCLASS()
class GWANG_FPS_API AFPSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;

	void StartNewGame(APlayerController* PlayerController);

	FTransform GetRandomPlayerStarts(ETeam Team);

	bool SpawnPlayer(APlayerController* PlayerController, ETeam Team);

	void OnPlayerDeath(APlayerController* PlayerController, ETeam Team);

	void FreePlayer(APlayerController* PlayerController);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTeamSelectionDelegate, ETeam, Team, bool, bCanJoinTeam);
	FTeamSelectionDelegate OnUpdateTeamSelectionUI;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FResetDelegate);
	FResetDelegate OnReset;

protected:
	void BeginPlay() override;

private:
	TArray<FTransform> MarvelTeamSpawnTransforms;
	TArray<FTransform> DCTeamSpawnTransforms;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AFPSCharacter> MarvelTeamCharacter;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AFPSCharacter> DCTeamCharacter;

	UPROPERTY(EditDefaultsOnly)
	int KillScoreToWin = 3;

	UPROPERTY(EditDefaultsOnly)
	int MaxPlayerPerTeam = 2;

	int CurrentMarvelScore;
	int CurrentDCScore;

	UPROPERTY()
	TArray<AFPSCharacter*> MarvelTeamPlayers;

	UPROPERTY()
	TArray<AFPSCharacter*> DCTeamPlayers;

private:
	void SetupPlayerStarts();
	void SetupPlayerPool();

	AFPSCharacter* PoolPlayer(ETeam Team);

	void CheckGameOver(APlayerController* PlayerController);
};

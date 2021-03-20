// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "FPSPlayerStart.h"

#include "FPSGameMode.generated.h"

class AFPSCharacter;
class AFPSGameStateBase;

UCLASS()
class GWANG_FPS_API AFPSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;

	void StartNewGame(APlayerController* PlayerController);

	FTransform GetRandomPlayerStarts(ETeam Team);

	void SpawnPlayer(APlayerController* PlayerController, ETeam Team);

	void OnPlayerDeath(APlayerController* PlayerController, ETeam Team);

	void FreePlayer(APawn* Player);

	bool CanJoin(ETeam Team);

	void CheckGameOver(int MarvelScore, int DCScore);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStartGameDelegate);
	FStartGameDelegate OnStartNewGame;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEndGameDelegate, ETeam, WinnerTeam);
	FEndGameDelegate OnEndGame;

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

	UPROPERTY()
	TArray<AFPSCharacter*> MarvelTeamPlayers;

	UPROPERTY()
	TArray<AFPSCharacter*> DCTeamPlayers;

private:
	UPROPERTY()
	AFPSGameStateBase* FPSGameState;

private:
	void SetupPlayerStarts();
	void SetupPlayerPool();

	AFPSCharacter* PoolPlayer(ETeam Team);
};

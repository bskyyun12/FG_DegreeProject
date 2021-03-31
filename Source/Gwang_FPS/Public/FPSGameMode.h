// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FPSGameInstance.h"
#include "FPSGameMode.generated.h"

class AFPSCharacter;
class AFPSGameState;

UCLASS()
class GWANG_FPS_API AFPSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGameModeDelegate);
	FGameModeDelegate OnStartGame;
	FGameModeDelegate OnStartMatch;
	FGameModeDelegate OnEndMatch;
	FGameModeDelegate OnEndGame;

	void StartGame();

	void StartMatch();

	FTransform GetRandomPlayerStarts(ETeam Team);

	void SpawnPlayer(APlayerController* PlayerController, ETeam Team);

	void OnPlayerDeath(APlayerController* PlayerController, ETeam Team);

	void FreePlayer(APawn* Player);

	ETeam GetWinnerTeam();

	void EndMatch(ETeam WinnerTeam);

	/////////////////
	// Doing this because I want to be able to play from FPS_Gwang map through engine!
	ETeam GetTeamWithLessPeople();
	/////////////////
	// Doing this because I want to be able to play from FPS_Gwang map through engine!

protected:
	void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	void PostLogin(APlayerController* NewPlayer) override;

	void BeginPlay() override;

private:
	TArray<FTransform> MarvelTeamSpawnTransforms;
	TArray<FTransform> DCTeamSpawnTransforms;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AFPSCharacter> MarvelTeamCharacter;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AFPSCharacter> DCTeamCharacter;

	UPROPERTY(EditDefaultsOnly)
	int MaxPlayersPerTeam = 6;

	UPROPERTY()
	TArray<AFPSCharacter*> MarvelPlayers;

	UPROPERTY()
	TArray<AFPSCharacter*> DCPlayers;

private:
	UPROPERTY()
	AFPSGameState* FPSGameState;

private:
	void SetupPlayerStarts();
	void SetupPlayerPool();

	AFPSCharacter* PoolPlayer(ETeam Team);
};

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

	void BeginPlay() override;

	bool SpawnPlayer(APlayerController* PlayerController, ETeam Team);

	FTransform GetRandomPlayerStarts(ETeam Team);

	// Game over
	void OnPlayerDeath(APlayerController* PlayerController, ETeam Team);

	bool CanJoin(ETeam Team);

	AFPSCharacter* PoolPlayer(ETeam Team);
	void FreePlayer(APlayerController* PlayerController);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTeamSelectionDelegate, ETeam, Team, bool, bCanJoinTeam);
	FTeamSelectionDelegate OnUpdateTeamSelectionUI;

private:
	TArray<FTransform> MarvelTeamSpawnTransforms;
	TArray<FTransform> DCTeamSpawnTransforms;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AFPSCharacter> MarvelTeamCharacter;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AFPSCharacter> DCTeamCharacter;

	void CheckGameOver(APlayerController* PlayerController);

private:
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
};

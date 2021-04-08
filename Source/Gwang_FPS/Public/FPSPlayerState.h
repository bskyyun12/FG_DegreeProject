// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PlayerStateInterface.h"
#include "FPSPlayerState.generated.h"

class AFPSPlayerController;

USTRUCT(BlueprintType)
struct FUserScore
{
	GENERATED_BODY();

	UPROPERTY()
	FName UserName;

	UPROPERTY()
	uint8 Kills;

	UPROPERTY()
	uint8 Deaths;

	FUserScore()
	{
		UserName = "Gwang";
		Kills = 0;
		Deaths = 0;
	}
};

UCLASS()
class GWANG_FPS_API AFPSPlayerState : public APlayerState, public IPlayerStateInterface
{
	GENERATED_BODY()

public:
	void Initialize(AFPSPlayerController* PC);

protected:
	AFPSPlayerController* PlayerController;

	FUserScore UserScore;

protected:
	UFUNCTION()
	void OnKill();

	UFUNCTION()
	void OnDeath();
};

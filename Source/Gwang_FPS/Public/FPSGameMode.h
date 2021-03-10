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

	void SpawnPlayer(APlayerController* PlayerController, ETeam Team);

	FTransform GetRandomPlayerStarts(ETeam Team);

private:
	TArray<FTransform> DarkCharacterSpawnTransforms;
	TArray<FTransform> SilverCharacterSpawnTransforms;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AFPSCharacter> DarkCharacter;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AFPSCharacter> SilverCharacter;

private:

};

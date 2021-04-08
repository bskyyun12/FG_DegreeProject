// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DeathMatchGameMode.generated.h"

class ADeathMatchPlayerController;

UCLASS()
class GWANG_FPS_API ADeathMatchGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:



protected:
	TArray<ADeathMatchPlayerController*> ActivePlayers;



protected:
	void PostLogin(APlayerController* NewPlayer) override;
	void Logout(AController* Exiting) override;

	void BeginPlay() override;

	void StartMatch();

};

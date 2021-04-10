// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DeathMatchPlayerController.generated.h"

class ADeathMatchGameMode;

UCLASS()
class GWANG_FPS_API ADeathMatchPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	ADeathMatchGameMode* GameMode;

protected:
	void BeginPlay() override;

	void OnPossess(APawn* aPawn) override;
	UFUNCTION(Client, Reliable)
	void Client_OnPossess();

};

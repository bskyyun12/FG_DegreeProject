// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPSGameModeBase.h"
#include "UserRow.h"
#include "LobbyGameMode.generated.h"

class ALobbyPlayerController;

UCLASS()
class GWANG_FPS_API ALobbyGameMode : public AFPSGameModeBase
{
	GENERATED_BODY()
	
public:
	void PostLogin(APlayerController* NewPlayer) override;

	void StartGame();

	void UpdateLobbyUI();

private:
	TArray<ALobbyPlayerController*> LobbyPlayerControllers;
	TArray<FUserRowData> UserData;
};

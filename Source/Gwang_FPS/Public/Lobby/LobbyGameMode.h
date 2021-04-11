// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "UserRow.h"

#include "LobbyGameMode.generated.h"

class ALobbyPlayerController;

UCLASS()
class GWANG_FPS_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	void PostLogin(APlayerController* NewPlayer) override;

	void StartGame();

	void GwangUpdateLobbyData(const FPlayerData& UpdatedData);
	void UpdateLobbyUI();

	void RemoveUserData(int ID);

protected:
	TArray<FPlayerData> UserData;

protected:
	void BeginPlay() override;

	int GetPlayerID(APlayerController* NewPlayer);
	ETeam GetTeamToJoin();
	FName GetUserName(APlayerController* NewPlayer);

	FTimerHandle LobbyTimer;
};

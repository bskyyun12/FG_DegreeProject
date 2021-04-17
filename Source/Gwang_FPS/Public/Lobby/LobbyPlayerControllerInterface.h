// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LobbyGameMode.h"
#include "FPSGameInstance.h"
#include "LobbyPlayerControllerInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class ULobbyPlayerControllerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GWANG_FPS_API ILobbyPlayerControllerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void LobbyToMainMenu();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void UpdateLobbyUI(const TArray<FLobbyPlayerData>& LobbyData);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void UpdateReadyStatus(bool bIsReady);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void UpdateTeamData(const ETeam& NewTeam);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnStartGame(const FLobbyPlayerData& LobbyPlayerData);

};

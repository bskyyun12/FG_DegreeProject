// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyInterface.h"
#include "UserRow.h"
#include "FPSGameInstance.h"
#include "LobbyPlayerController.generated.h"

class ALobbyGameMode;
class ULobbyWidget;
class UFPSGameInstance;

UCLASS()
class GWANG_FPS_API ALobbyPlayerController : public APlayerController, public ILobbyInterface
{
	GENERATED_BODY()

public:
	ALobbyPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// GameMode & UserData Setup
	void OnPostLogin_Implementation(ALobbyGameMode* LobbyGM, const FUserData& NewUserData) override;
	UFUNCTION(Client, Reliable)
	void Client_LoadLobbyWidget();

	// UserData Handle
	FUserData GetUserData_Implementation() override;
	void UpdateUserData_Implementation(const FUserData& NewData) override;
	UFUNCTION(Server, Reliable)
	void Server_UpdateUserdata(const FUserData& UpdatedData);
	UFUNCTION(Client, Reliable)
	void Client_UpdateUserdata(const FUserData& UpdatedData);

	// Lobby UI Updating
	void UpdateLobbyUI_Implementation(const TArray<FUserData>& UserDataList) override;
	UFUNCTION(Client, Reliable)
	void Client_UpdateLobbyUI(const TArray<FUserData>& UserDataList);

	// Lobby to MainMenu
	void LobbyToMainMenu_Implementation() override;
	UFUNCTION(Server, Reliable)
	void Server_LobbyToMainMenu();

	// Start Game
	void StartGame_Implementation() override;


private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> LobbyWidgetClass;

	ULobbyWidget* LobbyWidget;

	ALobbyGameMode* LobbyGameMode;

	UFPSGameInstance* GameInstance;

	UPROPERTY(Replicated)
	int ControllerID = 0;
};

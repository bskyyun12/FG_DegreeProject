// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UserRow.h"
#include "FPSGameInstance.h"
#include "LobbyPlayerControllerInterface.h"
#include "LobbyGameMode.h"
#include "LobbyPlayerController.generated.h"

class ALobbyGameMode;
class ULobbyWidget;
class UFPSGameInstance;

UCLASS()
class GWANG_FPS_API ALobbyPlayerController : public APlayerController, public ILobbyPlayerControllerInterface
{
	GENERATED_BODY()

public:
	ALobbyPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Called after ALobbyGameMode::UpdateLobbyUI()
	void UpdateLobbyUI_Implementation(const TArray<FLobbyPlayerData>& UserDataList) override;
	UFUNCTION(Client, Reliable)
	void Client_UpdateLobbyUI(const TArray<FLobbyPlayerData>& UserDataList);

	// Called after UUserRow::OnClicked_Button_Ready
	void UpdateReadyStatus_Implementation(bool bIsReady) override;
	UFUNCTION(Server, Reliable)
	void Server_UpdateReadyStatus(bool bIsReady);

	// Called after ULobbyWidget::OnClicked_Button_MarvelTeam, OnClicked_Button_DCTeam
	void UpdateTeamData_Implementation(const ETeam& NewTeam) override;
	UFUNCTION(Server, Reliable)
	void Server_UpdateTeamData(const ETeam& NewTeam);

	// Called after ULobbyWidget::OnClicked_Button_BackToMainMenu()
	void LobbyToMainMenu_Implementation() override;
	UFUNCTION(Server, Reliable)
	void Server_LobbyToMainMenu();

	// Called after ALobbyGameMode::StartGame()
	void OnStartGame_Implementation(const FLobbyPlayerData& LobbyPlayerData) override;
	UFUNCTION(Client, Reliable)
	void Client_OnStartGame(const FLobbyPlayerData& LobbyPlayerData);
	UFUNCTION(Server, Reliable)
	void Server_OnStartGame();

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> LobbyWidgetClass;

	ULobbyWidget* LobbyWidget;

	ALobbyGameMode* GM;

protected:
	void BeginPlay() override;
	UFUNCTION(Server, Reliable)
	void Server_OnSetupWidget();
};

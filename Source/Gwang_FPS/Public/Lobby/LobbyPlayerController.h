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

	void StartGame_Implementation() override;

	void LoadLobbyWidget_Implementation() override;
	UFUNCTION(Client, Reliable)
	void Client_LoadLobbyWidget();

	void SetControllerID_Implementation(int ID) override;

	int GetControllerID_Implementation() override;

	void SetIsReady_Implementation(bool bIsReady) override;

	void SetLobbyGameMode_Implementation(ALobbyGameMode* LobbyGM) override;

	void RequestLobbyUIUpdate_Implementation() override;

	void LobbyToMainMenu_Implementation() override;
	UFUNCTION(Server, Reliable)
	void Server_LobbyToMainMenu();

	void UpdateLobbyUI_Implementation(const TArray<FUserData>& UserData) override;
	UFUNCTION(Client, Reliable)
	void Client_UpdateLobbyUI(const TArray<FUserData>& UserData);

	void UpdateLobbyData_Implementation(ETeam LobbyTeam) override;
	UFUNCTION(Server, Reliable)
	void Server_UpdateLobbyData(ETeam LobbyTeam);
	UFUNCTION(Client, Reliable)
	void Client_UpdateLobbyData(ETeam LobbyTeam);

protected:
	void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void Server_UpdateLobbyUI();
	UFUNCTION(Server, Reliable)
	void Server_UpdateReadyStatus(bool bIsReady);

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> LobbyWidgetClass;

	ULobbyWidget* LobbyWidget;

	ALobbyGameMode* LobbyGameMode;

	UFPSGameInstance* GameInstance;

	UPROPERTY(Replicated)
	int ControllerID = 0;
};

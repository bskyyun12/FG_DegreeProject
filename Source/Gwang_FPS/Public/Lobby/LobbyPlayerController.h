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

	void UpdateLobbyUI_Implementation(const TArray<FUserRowData>& UserRowData) override;
	UFUNCTION(Client, Reliable)
	void Client_UpdateLobbyUI(const TArray<FUserRowData>& UserRowData);

	void SetControllerID_Implementation(int ID) override;

	int GetControllerID_Implementation() override;

	void SetIsReady_Implementation(bool bIsReady) override;

	void SetTeam_Implementation(ETeam Team) override;

	void SetLobbyGameMode_Implementation(ALobbyGameMode* LobbyGM) override;

	void RequestLobbyUIUpdate_Implementation() override;

protected:
	void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void Server_UpdateLobbyUI();
	UFUNCTION(Server, Reliable)
	void Server_UpdateReadyStatus(bool bIsReady);
	UFUNCTION(Server, Reliable)
	void Server_UpdateTeamStatus(ETeam Team);
	UFUNCTION(Client, Reliable)
	void Client_UpdateTeamStatus(ETeam Team);

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> LobbyWidgetClass;

	ULobbyWidget* LobbyWidget;

	ALobbyGameMode* LobbyGameMode;

	UPROPERTY(Replicated)
	int ControllerID = 0;
};

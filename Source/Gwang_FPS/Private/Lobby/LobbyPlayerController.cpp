// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyPlayerController.h"

#include "Lobby/LobbyWidget.h"
#include "Lobby/UserRow.h"
#include "Lobby/LobbyGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "FPSGameInstance.h"

ALobbyPlayerController::ALobbyPlayerController(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
{
	bReplicates = true;
}

void ALobbyPlayerController::BeginPlay()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		// Cache GameMode
		UWorld* World = GetWorld();
		if (World != nullptr)
		{
			GM = World->GetAuthGameMode<ALobbyGameMode>();
			if (!ensure(GM != nullptr))
			{
				return;
			}
		}
	}

	if (IsLocalController())
	{
		UWorld* World = GetWorld();
		if (!ensure(World != nullptr))
		{
			return;
		}
		if (!ensure(LobbyWidgetClass != nullptr))
		{
			return;
		}

		LobbyWidget = CreateWidget<ULobbyWidget>(World, LobbyWidgetClass);
		if (!ensure(LobbyWidget != nullptr))
		{
			return;
		}
		LobbyWidget->Setup();

		Server_OnSetupWidget();
	}
}

void ALobbyPlayerController::Server_OnSetupWidget_Implementation()
{
	if (GM != nullptr)
	{
		GM->UpdateLobbyUI();
	}
}

// Called after ALobbyGameMode::UpdateLobbyUI()
void ALobbyPlayerController::UpdateLobbyUI_Implementation(const TArray<FLobbyPlayerData>& UserDataList)
{
	Client_UpdateLobbyUI(UserDataList);
}

void ALobbyPlayerController::Client_UpdateLobbyUI_Implementation(const TArray<FLobbyPlayerData>& UserDataList)
{
	LobbyWidget->UpdateUserRowData(UserDataList);
}

// ( bIsReady Update ) Called after UUserRow::OnClicked_Button_Ready
void ALobbyPlayerController::UpdateReadyStatus_Implementation(bool bIsReady)
{
	Server_UpdateReadyStatus(bIsReady);
}

void ALobbyPlayerController::Server_UpdateReadyStatus_Implementation(bool bIsReady)
{
	if (GM != nullptr)
	{
		FLobbyPlayerData LobbyData = GM->GetLobbyPlayerData(this);
		LobbyData.bIsReady = bIsReady;
		GM->UpdateLobbyPlayerData(LobbyData);
	}
}

// ( Team Update ) Called after ULobbyWidget::OnClicked_Button_MarvelTeam, OnClicked_Button_DCTeam
void ALobbyPlayerController::UpdateTeamData_Implementation(const ETeam& NewTeam)
{
	Server_UpdateTeamData(NewTeam);
}

void ALobbyPlayerController::Server_UpdateTeamData_Implementation(const ETeam& NewTeam)
{
	if (GM != nullptr)
	{
		FLobbyPlayerData LobbyData = GM->GetLobbyPlayerData(this);
		LobbyData.Team = NewTeam;
		GM->UpdateLobbyPlayerData(LobbyData);
	}
}

// Called after ULobbyWidget::OnClicked_Button_BackToMainMenu()
void ALobbyPlayerController::LobbyToMainMenu_Implementation()
{
	UFPSGameInstance* GI = Cast<UFPSGameInstance>(GetGameInstance());
	if (!ensure(GI != nullptr))
	{
		GI->DestroySession();
		UGameplayStatics::OpenLevel(GetWorld(), TEXT("Menu"));
		Server_LobbyToMainMenu();
	}
}

void ALobbyPlayerController::Server_LobbyToMainMenu_Implementation()
{
	if (GM != nullptr)
	{
		GM->RemoveLobbyPlayerData(this);
	}
}

void ALobbyPlayerController::OnStartGame_Implementation(const FLobbyPlayerData& LobbyPlayerData)
{
	Client_OnStartGame(LobbyPlayerData);
}

void ALobbyPlayerController::Client_OnStartGame_Implementation(const FLobbyPlayerData& LobbyPlayerData)
{
	UFPSGameInstance* GI = Cast<UFPSGameInstance>(GetGameInstance());
	if (!ensure(GI != nullptr))
	{
		return;
	}

	FPlayerData PlayerData = GI->GetPlayerData();

	PlayerData.PlayerName = LobbyPlayerData.PlayerName;
	PlayerData.Team = LobbyPlayerData.Team;

	// Weapons are handled directly in ( ULobbyInventory::OnClicked_Button_Apply ) because players don't need to know each others' weapons
	//PlayerData.StartMainWeapon = LobbyPlayerData.StartMainWeapon;
	//PlayerData.StartSubWeapon = LobbyPlayerData.StartSubWeapon;
	//PlayerData.StartMeleeWeapon = LobbyPlayerData.StartMeleeWeapon;
	//PlayerData.StartGrenade = LobbyPlayerData.StartGrenade;

	GI->SetPlayerData(PlayerData);

	UE_LOG(LogTemp, Warning, TEXT("ALobbyPlayerController::Client_OnStartGame ( %i ) => ( %s ) team: %i, StartMain: %i"), GetLocalRole(), *GetName(), GI->GetPlayerData().Team, GI->GetPlayerData().StartMainWeapon);

	Server_OnStartGame();
}

void ALobbyPlayerController::Server_OnStartGame_Implementation()
{
	if (GM != nullptr)
	{
		FLobbyPlayerData LobbyData = GM->GetLobbyPlayerData(this);
		LobbyData.bFinishedSavingData = true;
		GM->UpdateLobbyPlayerData(LobbyData);
	}
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyPlayerController.h"
#include "Net/UnrealNetwork.h"

#include "Lobby/LobbyWidget.h"
#include "Lobby/UserRow.h"
#include "Lobby/LobbyGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "FPSGameInstance.h"

void ALobbyPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALobbyPlayerController, ControllerID);
}

ALobbyPlayerController::ALobbyPlayerController(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
{
	bReplicates = true;
}

void ALobbyPlayerController::LoadLobbyWidget_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("ALobbyPlayerController::LoadLobbyWidget_Implementation"));
	Client_LoadLobbyWidget();
}

void ALobbyPlayerController::Client_LoadLobbyWidget_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("ALobbyPlayerController::Client_LoadLobbyWidget_Implementation"));
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
}

void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	RequestLobbyUIUpdate_Implementation();
}

void ALobbyPlayerController::RequestLobbyUIUpdate_Implementation()
{
	Server_UpdateLobbyUI();
}

void ALobbyPlayerController::Server_UpdateLobbyUI_Implementation()
{
	if (LobbyGameMode != nullptr)
	{
		LobbyGameMode->UpdateLobbyUI();
	}
}

void ALobbyPlayerController::SetIsReady_Implementation(bool bIsReady)
{
	Server_UpdateReadyStatus(bIsReady);
}

void ALobbyPlayerController::Server_UpdateReadyStatus_Implementation(bool bIsReady)
{
	if (LobbyGameMode != nullptr)
	{
		LobbyGameMode->UpdateReadyStatus(ControllerID, bIsReady);
	}
}

void ALobbyPlayerController::SetTeam_Implementation(ETeam Team)
{
	Server_UpdateTeamStatus(Team);
	Client_UpdateTeamStatus(Team);
}

void ALobbyPlayerController::Server_UpdateTeamStatus_Implementation(ETeam Team)
{
	if (LobbyGameMode != nullptr)
	{
		LobbyGameMode->UpdateTeamStatus(ControllerID, Team);
	}
}

void ALobbyPlayerController::Client_UpdateTeamStatus_Implementation(ETeam Team)
{
	UFPSGameInstance* GameInstance = Cast<UFPSGameInstance>(GetGameInstance());
	if (GameInstance != nullptr)
	{
		GameInstance->SetTeam(Team);
		UE_LOG(LogTemp, Warning, TEXT("----------------------------------------------"));
		switch (GameInstance->GetTeam())
		{
		case ETeam::None:
			UE_LOG(LogTemp, Warning, TEXT("(Client)ID: %i, Team: ETeam::None"), ControllerID);
			break;
		case ETeam::Marvel:
			UE_LOG(LogTemp, Warning, TEXT("(Client)ID: %i, Team: ETeam::Marvel"), ControllerID);
			break;
		case ETeam::DC:
			UE_LOG(LogTemp, Warning, TEXT("(Client)ID: %i, Team: ETeam::DC"), ControllerID);
			break;
		}
		UE_LOG(LogTemp, Warning, TEXT("----------------------------------------------"));
	}
}

void ALobbyPlayerController::UpdateLobbyUI_Implementation(const TArray<FUserRowData>& UserRowData)
{
	UE_LOG(LogTemp, Warning, TEXT("ALobbyPlayerController::UpdateLobbyUI_Implementation"));
	Client_UpdateLobbyUI(UserRowData);
}

void ALobbyPlayerController::Client_UpdateLobbyUI_Implementation(const TArray<FUserRowData>& UserData)
{
	UE_LOG(LogTemp, Warning, TEXT("ALobbyPlayerController::Client_UpdateLobbyUI_Implementation"));
	LobbyWidget->UpdateUserRowData(UserData);
}

void ALobbyPlayerController::SetLobbyGameMode_Implementation(ALobbyGameMode* LobbyGM)
{
	if (HasAuthority())
	{
		LobbyGameMode = LobbyGM;
	}
}

int ALobbyPlayerController::GetControllerID_Implementation()
{
	return ControllerID;
}

void ALobbyPlayerController::SetControllerID_Implementation(int ID)
{
	ControllerID = ID;
}

void ALobbyPlayerController::StartGame_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("ALobbyPlayerController::StartGame_Implementation"));
	if (HasAuthority())
	{
		if (LobbyGameMode != nullptr)
		{
			LobbyGameMode->StartGame();
		}
	}
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyPlayerController.h"
#include "Net/UnrealNetwork.h"

#include "Lobby/LobbyWidget.h"
#include "Lobby/UserRow.h"
#include "Lobby/LobbyGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "FPSGameInstance.h"

ALobbyPlayerController::ALobbyPlayerController(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
{
	bReplicates = true;
}

void ALobbyPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALobbyPlayerController, ControllerID);
}

void ALobbyPlayerController::OnPostLogin_Implementation(ALobbyGameMode* LobbyGM, const FPlayerData& NewUserData)
{
	LobbyGameMode = LobbyGM;
	ControllerID = NewUserData.ControllerID; 
	Client_LoadLobbyWidget();
	Execute_UpdateUserData(this, NewUserData);
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

#pragma region UserData Handle
FPlayerData ALobbyPlayerController::GetUserData_Implementation()
{
	if (GameInstance == nullptr)
	{
		GameInstance = Cast<UFPSGameInstance>(GetGameInstance());
		if (!ensure(GameInstance != nullptr))
		{
			return FPlayerData();
		}
	}
	return GameInstance->GetUserData();
}

void ALobbyPlayerController::UpdateUserData_Implementation(const FPlayerData& NewData)
{
	Server_UpdateUserdata(NewData);
	Client_UpdateUserdata(NewData);
}

void ALobbyPlayerController::Server_UpdateUserdata_Implementation(const FPlayerData& UpdatedData)
{
	if (LobbyGameMode != nullptr)
	{
		LobbyGameMode->GwangUpdateLobbyData(UpdatedData);
	}
}

void ALobbyPlayerController::Client_UpdateUserdata_Implementation(const FPlayerData& UpdatedData)
{
	if (GameInstance == nullptr)
	{
		GameInstance = Cast<UFPSGameInstance>(GetGameInstance());
		if (!ensure(GameInstance != nullptr))
		{
			return;
		}
	}
	GameInstance->SetUserData(UpdatedData);
}
#pragma endregion UserData Handle

// Called by ALobbyGameMode::UpdateLobbyUI
void ALobbyPlayerController::UpdateLobbyUI_Implementation(const TArray<FPlayerData>& UserDataList)
{
	Client_UpdateLobbyUI(UserDataList);
}

void ALobbyPlayerController::Client_UpdateLobbyUI_Implementation(const TArray<FPlayerData>& UserDataList)
{
	LobbyWidget->UpdateUserRowData(UserDataList);
}

void ALobbyPlayerController::LobbyToMainMenu_Implementation()
{
	GameInstance->DestroySession();
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("Menu"));
	Server_LobbyToMainMenu();
}

void ALobbyPlayerController::Server_LobbyToMainMenu_Implementation()
{
	LobbyGameMode->RemoveUserData(ControllerID);
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
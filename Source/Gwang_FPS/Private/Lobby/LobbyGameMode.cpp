// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerState.h"

#include "Lobby/LobbyPlayerController.h"
#include "FPSGameInstance.h"
#include "Lobby/LobbyPlayerControllerInterface.h"


void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// TODO: Make Logout and remove the UserData
	FLobbyPlayerData Data;
	Data.PlayerName = GetUserName(NewPlayer);
	Data.Team = GetTeamToJoin();
	Data.bIsReady = false;
	Data.StartMainWeapon = EMainWeapon::M4A1;
	Data.StartSubWeapon = ESubWeapon::Pistol;
	Data.StartMeleeWeapon = EMeleeWeapon::Knife;
	Data.StartGrenade = EGrenade::Grenade;
	Data.PlayerController = NewPlayer;
	LobbyPlayerData.Add(Data);

	PlayerControllers.Add(NewPlayer);
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	PlayerControllers.Remove(Cast<APlayerController>(Exiting));
}

FLobbyPlayerData ALobbyGameMode::GetLobbyPlayerData(APlayerController* PlayerController) const
{
	for (const FLobbyPlayerData& Data : LobbyPlayerData)
	{
		if (Data.PlayerController == PlayerController)
		{
			return Data;
		}
	}

	return FLobbyPlayerData();
}

// Called after ALobbyPlayerController::Server_UpdateReadyStatus
void ALobbyGameMode::UpdateLobbyPlayerData(const FLobbyPlayerData& UpdatedData)
{
	for (FLobbyPlayerData& Data : LobbyPlayerData)
	{
		if (Data.PlayerController == UpdatedData.PlayerController)
		{
			Data = UpdatedData;
			UE_LOG(LogTemp, Warning, TEXT("ALobbyGameMode::UpdateLobbyData => ( %s ) Data Updated!"), *Data.PlayerController->GetName());
			break;
		}
	}

	UpdateLobbyUI();
}

void ALobbyGameMode::UpdateLobbyUI()
{
	UE_LOG(LogTemp, Warning, TEXT("ALobbyGameMode::UpdateLobbyUI"));
	for (APlayerController* PC : PlayerControllers)
	{
		if (PC != nullptr && UKismetSystemLibrary::DoesImplementInterface(PC, ULobbyPlayerControllerInterface::StaticClass()))
		{
			ILobbyPlayerControllerInterface::Execute_UpdateLobbyUI(PC, LobbyPlayerData);
		}
	}

	if (!bAllPlayerReady && IsReadyToStartGame())
	{
		StartGame();
	}
}

bool ALobbyGameMode::IsReadyToStartGame()
{
	for (const FLobbyPlayerData& Data : LobbyPlayerData)
	{
		if (Data.bIsReady == false)
		{
			return false;
		}
	}

	bAllPlayerReady = true;
	return true;
}

void ALobbyGameMode::StartGame()
{
	UE_LOG(LogTemp, Warning, TEXT("ALobbyGameMode::StartGame"));

	// Call OnStartGame on all players, then players will 
	for (const FLobbyPlayerData& Data : LobbyPlayerData)
	{
		if (Data.PlayerController != nullptr && UKismetSystemLibrary::DoesImplementInterface(Data.PlayerController, ULobbyPlayerControllerInterface::StaticClass()))
		{
			ILobbyPlayerControllerInterface::Execute_OnStartGame(Data.PlayerController, Data);
		}
	}

	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		World->GetTimerManager().SetTimer(GameStartTimer, this, &ALobbyGameMode::GameStartCheck, 1.f, true);
	}
}

void ALobbyGameMode::GameStartCheck()
{
	for (const FLobbyPlayerData& Data : LobbyPlayerData)
	{
		if (Data.bFinishedSavingData == false)
		{
			UE_LOG(LogTemp, Warning, TEXT("ALobbyGameMode::GameStartCheck => ( %s ) is not ready to travel yet"), *Data.PlayerController->GetName());
			return;
		}
	}

	if (GEngine != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "Game Start!");
	}

	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		World->GetTimerManager().ClearTimer(GameStartTimer);

		UFPSGameInstance* GameInstance = Cast<UFPSGameInstance>(GetGameInstance());
		if (GameInstance != nullptr)
		{
			GameInstance->StartSession();
		}

		//bUseSeamlessTravel = true;
		World->ServerTravel("/Game/Maps/Gwang_FPS?listen");
	}
}

void ALobbyGameMode::RemoveLobbyPlayerData(APlayerController* PlayerController)
{
	int Index = 0;
	for (const FLobbyPlayerData& Data : LobbyPlayerData)
	{
		if (Data.PlayerController == PlayerController)
		{
			break;
		}
		Index++;
	}
	LobbyPlayerData.RemoveAt(Index);
}

ETeam ALobbyGameMode::GetTeamToJoin()
{
	int MarvelTeamCounter = 0;
	int DCTeamCounter = 0;
	for (const FLobbyPlayerData& Data : LobbyPlayerData)
	{
		if (Data.Team == ETeam::Marvel)
		{
			MarvelTeamCounter++;
		}
		else if (Data.Team == ETeam::DC)
		{
			DCTeamCounter++;
		}
	}
	return MarvelTeamCounter <= DCTeamCounter ? ETeam::Marvel : ETeam::DC;
}

FName ALobbyGameMode::GetUserName(APlayerController* NewPlayer) const
{
	APlayerState* PlayerState = NewPlayer->GetPlayerState<APlayerState>();
	FName UserName = "Gwang";
	if (PlayerState != nullptr)
	{
		UserName = *PlayerState->GetPlayerName();
	}
	return UserName;
}

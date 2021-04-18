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
	UE_LOG(LogTemp, Warning, TEXT("ALobbyGameMode::PostLogin => ( %s ) logged in!"), *NewPlayer->GetName());

	FLobbyPlayerData Data;
	Data.Id = GetPlayerId(NewPlayer);
	Data.PlayerName = GetPlayerName(NewPlayer);
	Data.Team = GetTeamToJoin();
	Data.bIsReady = false;
	Data.StartMainWeapon = EMainWeapon::M4A1;
	Data.StartSubWeapon = ESubWeapon::Pistol;
	Data.StartMeleeWeapon = EMeleeWeapon::Knife;
	Data.StartGrenade = EGrenade::Grenade;
	LobbyPlayerData.Add(Data);

	PlayerControllers.Add(NewPlayer);

	UE_LOG(LogTemp, Warning, TEXT("ALobbyGameMode::PostLogin => NumPlayers: %i"), PlayerControllers.Num());
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	UE_LOG(LogTemp, Warning, TEXT("ALobbyGameMode::Logout"));
	APlayerController* PlayerController = Cast<APlayerController>(Exiting);
	if (!ensure(PlayerController != nullptr))
	{
		return;
	}

	uint8 Index = 0;
	for (uint8 i = 0; i < LobbyPlayerData.Num(); i++)
	{
		if (LobbyPlayerData[i].Id == GetPlayerId(PlayerController))
		{
			Index = i;
			break;
		}
	}
	if (Index != 0)	// not server player
	{
		LobbyPlayerData.RemoveAt(Index);
	}
	PlayerControllers.Remove(PlayerController);

	UpdateLobbyUI();
	Super::Logout(Exiting);
}

FLobbyPlayerData ALobbyGameMode::GetLobbyPlayerData(APlayerController* PlayerController) const
{
	for (const FLobbyPlayerData& Data : LobbyPlayerData)
	{
		if (Data.Id == GetPlayerId(PlayerController))
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
		if (Data.Id == UpdatedData.Id)
		{
			Data = UpdatedData;
			UE_LOG(LogTemp, Warning, TEXT("ALobbyGameMode::UpdateLobbyData => ( %s ) Data Updated!"), *Data.PlayerName.ToString());
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

	// Call OnStartGame on all players, then players will save data in their GameInstance
	for (APlayerController* PC : PlayerControllers)
	{
		if (PC != nullptr && UKismetSystemLibrary::DoesImplementInterface(PC, ULobbyPlayerControllerInterface::StaticClass()))
		{
			ILobbyPlayerControllerInterface::Execute_OnStartGame(PC);
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
			UE_LOG(LogTemp, Warning, TEXT("ALobbyGameMode::GameStartCheck => ( %s ) is not ready to travel yet"), *Data.PlayerName.ToString());
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

FName ALobbyGameMode::GetPlayerName(APlayerController* NewPlayer) const
{
	APlayerState* PlayerState = NewPlayer->GetPlayerState<APlayerState>();
	FName UserName = "Gwang";
	if (PlayerState != nullptr)
	{
		UserName = *PlayerState->GetPlayerName();
	}
	return UserName;
}

int32 ALobbyGameMode::GetPlayerId(APlayerController* NewPlayer) const
{
	APlayerState* PlayerState = NewPlayer->GetPlayerState<APlayerState>();
	if (PlayerState != nullptr)
	{
		return PlayerState->GetPlayerId();
	}
	return -1;
}

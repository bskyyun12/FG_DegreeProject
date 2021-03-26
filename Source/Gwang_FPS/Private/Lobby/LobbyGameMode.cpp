// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerState.h"

#include "Lobby/LobbyPlayerController.h"
#include "FPSGameInstance.h"
#include "Lobby/LobbyInterface.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	UE_LOG(LogTemp, Warning, TEXT("ALobbyGameMode::PostLogin"));

	if (NewPlayer == nullptr)
	{
		return;
	}

	FUserRowData NewUser;
	//NewUser.UserName = *NewPlayer->GetName();
	NewUser.UserName = GetUserName(NewPlayer);
	NewUser.ControllerID = GetPlayerID(NewPlayer);
	NewUser.Team = GetTeamToJoin();
	UserData.Add(NewUser);

	if (UKismetSystemLibrary::DoesImplementInterface(NewPlayer, ULobbyInterface::StaticClass()))
	{
		ILobbyInterface::Execute_SetLobbyGameMode(NewPlayer, this);
		ILobbyInterface::Execute_SetControllerID(NewPlayer, NewUser.ControllerID);
		ILobbyInterface::Execute_SetTeam(NewPlayer, NewUser.Team);
		ILobbyInterface::Execute_LoadLobbyWidget(NewPlayer);
	}
}

void ALobbyGameMode::StartGame()
{
	UFPSGameInstance* GameInstance = Cast<UFPSGameInstance>(GetGameInstance());

	if (GameInstance != nullptr)
	{
		GameInstance->StartSession();
	}

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	//bUseSeamlessTravel = true;
	World->ServerTravel("/Game/Maps/Gwang_FPS?listen");
}

void ALobbyGameMode::UpdateLobbyUI()
{
	UE_LOG(LogTemp, Warning, TEXT("ALobbyGameMode::UpdateLobbyUI"));

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController != nullptr && UKismetSystemLibrary::DoesImplementInterface(PlayerController, ULobbyInterface::StaticClass()))
		{
			ILobbyInterface::Execute_UpdateLobbyUI(PlayerController, UserData);
		}
	}
}

void ALobbyGameMode::UpdateReadyStatus(int ID, bool bIsReady)
{
	for (FUserRowData& Data : UserData)
	{
		if (Data.ControllerID == ID)
		{
			Data.bIsReady = bIsReady;
			break;
		}
	}
}

void ALobbyGameMode::UpdateTeamStatus(int ID, ETeam Team)
{
	for (FUserRowData& Data : UserData)
	{
		if (Data.ControllerID == ID)
		{
			Data.Team = Team;
			break;
		}
	}
}

int ALobbyGameMode::GetPlayerID(APlayerController* NewPlayer)
{
	APlayerState* PlayerState = NewPlayer->GetPlayerState<APlayerState>();
	int NewPlayerID = -1;
	if (PlayerState != nullptr)
	{
		NewPlayerID = PlayerState->GetPlayerId();
	}
	return NewPlayerID;
}

ETeam ALobbyGameMode::GetTeamToJoin()
{
	int MarvelTeamCounter = 0;
	int DCTeamCounter = 0;
	for (FUserRowData& Data : UserData)
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

FName ALobbyGameMode::GetUserName(APlayerController* NewPlayer)
{
	APlayerState* PlayerState = NewPlayer->GetPlayerState<APlayerState>();
	FName UserName = "Gwang";
	if (PlayerState != nullptr)
	{
		UserName = *PlayerState->GetPlayerName();
	}
	return UserName;
}

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

	FPlayerData NewUserData;
	NewUserData.UserName = GetUserName(NewPlayer);
	NewUserData.ControllerID = GetPlayerID(NewPlayer);
	NewUserData.bIsReady = false;
	NewUserData.Team = GetTeamToJoin();
	NewUserData.StartMainWeapon = EMainWeapon::M4A1;
	NewUserData.StartSubWeapon = ESubWeapon::Pistol;
	NewUserData.StartKnife = EKnife::Knife;
	NewUserData.StartGrenade = EGrenade::Grenade;
	UserData.Add(NewUserData);

	if (UKismetSystemLibrary::DoesImplementInterface(NewPlayer, ULobbyInterface::StaticClass()))
	{
		ILobbyInterface::Execute_OnPostLogin(NewPlayer, this, NewUserData);
	}
}

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	World->GetTimerManager().SetTimer(LobbyTimer, [&](){ UpdateLobbyUI(); }, .5f, true);
}

void ALobbyGameMode::StartGame()
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	World->GetTimerManager().ClearTimer(LobbyTimer);

	UFPSGameInstance* GameInstance = Cast<UFPSGameInstance>(GetGameInstance());
	if (GameInstance != nullptr)
	{
		GameInstance->StartSession();
	}

	//bUseSeamlessTravel = true;
	World->ServerTravel("/Game/Maps/Gwang_FPS?listen");
}

void ALobbyGameMode::GwangUpdateLobbyData(const FPlayerData& UpdatedData)
{
	UE_LOG(LogTemp, Warning, TEXT("ALobbyGameMode::GwangUpdateLobbyData"));

	for (FPlayerData& Data : UserData)
	{
		if (Data == UpdatedData)
		{
			Data = UpdatedData;
			UE_LOG(LogTemp, Warning, TEXT("Data Updated!"));
			break;
		}
	}
}

void ALobbyGameMode::UpdateLobbyUI()
{
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

void ALobbyGameMode::RemoveUserData(int ID)
{
	int Index = 0;
	for (FPlayerData& Data : UserData)
	{
		if (Data.ControllerID == ID)
		{
			break;
		}
		Index++;
	}
	UserData.RemoveAt(Index);
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
	for (FPlayerData& Data : UserData)
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

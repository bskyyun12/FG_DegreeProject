// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyGameMode.h"

#include "Lobby/LobbyPlayerController.h"
#include "MainMenu/FPSGameInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Lobby/LobbyInterface.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	UE_LOG(LogTemp, Warning, TEXT("ALobbyGameMode::PostLogin"));

	ALobbyPlayerController* LobbyPlayerController = Cast<ALobbyPlayerController>(NewPlayer);
	LobbyPlayerControllers.Add(LobbyPlayerController);

	FUserRowData Data;
	Data.UserName = LobbyPlayerController->GetName();
	UserData.Add(Data);
}

void ALobbyGameMode::StartGame()
{
	auto GameInstance = Cast<UFPSGameInstance>(GetGameInstance());

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
	for (FUserRowData Data : UserData)
	{
		UE_LOG(LogTemp, Warning, TEXT("Data.UserName: %s"), *Data.UserName);
	}

	for (ALobbyPlayerController* LobbyPlayerController : LobbyPlayerControllers)
	{
		LobbyPlayerController->Server_UpdateLobbyUI(UserData);
	}
}
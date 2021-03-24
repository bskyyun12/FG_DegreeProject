// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyPlayerController.h"

#include "Lobby/LobbyWidget.h"
#include "Lobby/UserRow.h"
#include "Lobby/LobbyGameMode.h"
#include <Kismet/GameplayStatics.h>

void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("ALobbyPlayerController::BeginPlay"));

	Client_LoadLobbyWidget();
	Server_OnUserEntered();
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

void ALobbyPlayerController::Server_OnUserEntered_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("ALobbyPlayerController::OnUserEntered_Implementation"));

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	ALobbyGameMode* LobbyGameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(World));
	if (!ensure(LobbyGameMode != nullptr))
	{
		return;
	}
	LobbyGameMode->UpdateLobbyUI();
}

void ALobbyPlayerController::Server_UpdateLobbyUI_Implementation(const TArray<FUserRowData>& UserRowData)
{
	UE_LOG(LogTemp, Warning, TEXT("ALobbyPlayerController::Server_UpdateLobbyUI_Implementation"));
	Client_UpdateLobbyUI(UserRowData);
}

void ALobbyPlayerController::Client_UpdateLobbyUI_Implementation(const TArray<FUserRowData>& UserData)
{
	UE_LOG(LogTemp, Warning, TEXT("ALobbyPlayerController::Client_UpdateLobbyUI_Implementation"));
	LobbyWidget->UpdateUserRowData(UserData);
}

void ALobbyPlayerController::StartGame_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("ALobbyPlayerController::StartGame_Implementation"));
	ALobbyGameMode* LobbyGameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	LobbyGameMode->StartGame();
}
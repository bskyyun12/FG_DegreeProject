// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchGameMode.h"

#include "DeathMatchPlayerController.h"

void ADeathMatchGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	ActivePlayers.Add(Cast<ADeathMatchPlayerController>(NewPlayer));
}

void ADeathMatchGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	ActivePlayers.Remove(Cast<ADeathMatchPlayerController>(Exiting));
}

void ADeathMatchGameMode::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchGameMode::BeginPlay"));
}

void ADeathMatchGameMode::StartMatch()
{
	
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSGameState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h"

#include "FPSGameMode.h"
#include "FPSPlayerControllerInterface.h"

void AFPSGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSGameState, MarvelScore);
	DOREPLIFETIME(AFPSGameState, DCScore);
}

void AFPSGameState::Init(AFPSGameMode* GameMode)
{
	FPSGameMode = GameMode;
	FPSGameMode->OnStartMatch.AddDynamic(this, &AFPSGameState::OnStartMatch);
	FPSGameMode->OnEndMatch.AddDynamic(this, &AFPSGameState::OnEndMatch);
}

void AFPSGameState::OnStartMatch()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSGameState::OnStartMatch()"));
}

void AFPSGameState::OnEndMatch()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSGameState::OnEndMatch()"));

	ETeam WinnerTeam = FPSGameMode->GetWinnerTeam();
	if (WinnerTeam == ETeam::Marvel)
	{
		MarvelScore++;	// OnRep_MarvelScore()
	}
	else if (WinnerTeam == ETeam::DC)
	{
		DCScore++;	// OnRep_DCScore()
	}

	OnUpdateScore.Broadcast(MarvelScore, DCScore);
}

void AFPSGameState::OnRep_MarvelScore()
{
	OnUpdateScore.Broadcast(MarvelScore, DCScore);
}

void AFPSGameState::OnRep_DCScore()
{
	OnUpdateScore.Broadcast(MarvelScore, DCScore);
}


// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSGameStateBase.h"
#include "Net/UnrealNetwork.h"

#include "FPSGameMode.h"


void AFPSGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSGameStateBase, MarvelScore);
	DOREPLIFETIME(AFPSGameStateBase, DCScore);
}

void AFPSGameStateBase::Initialize(AFPSGameMode* GameMode)
{
	FPSGameMode = GameMode;
	if (!ensure(FPSGameMode != nullptr))
	{
		return;
	}

	FPSGameMode->OnStartNewGame.AddDynamic(this, &AFPSGameStateBase::OnStartNewGame);
}

void AFPSGameStateBase::OnStartNewGame()
{
	MarvelScore = 0;
	DCScore = 0;
}

void AFPSGameStateBase::OnPlayerDeath(ETeam Team)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSGameStateBase::OnPlayerDeath()"));
	if (HasAuthority())
	{
		if (Team == ETeam::Marvel)
		{
			DCScore++;
		}
		else if (Team == ETeam::DC)
		{
			MarvelScore++;
		}
		UE_LOG(LogTemp, Warning, TEXT("Server: MarvelScore(%i), DCScore(%i)"), MarvelScore, DCScore);

		if (FPSGameMode != nullptr)
		{
			FPSGameMode->CheckGameOver(MarvelScore, DCScore);
		}
	}
}
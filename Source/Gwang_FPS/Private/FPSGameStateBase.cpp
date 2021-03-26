// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSGameStateBase.h"
#include "Net/UnrealNetwork.h"

#include "FPSGameMode.h"

void AFPSGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSGameStateBase, MarvelTeamRoundScore);
	DOREPLIFETIME(AFPSGameStateBase, DCTeamRoundScore);
}

void AFPSGameStateBase::OnStartNewGame()
{
	MarvelTeamRoundScore = 0;
	DCTeamRoundScore = 0;
}

void AFPSGameStateBase::OnPlayerDeath(ETeam Team)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSGameStateBase::OnPlayerDeath()"));
	if (HasAuthority())
	{
		if (Team == ETeam::Marvel)
		{
			DCTeamRoundScore++;
		}
		else if (Team == ETeam::DC)
		{
			MarvelTeamRoundScore++;
		}
		UE_LOG(LogTemp, Warning, TEXT("Server: MarvelScore(%i), DCScore(%i)"), MarvelTeamRoundScore, DCTeamRoundScore);
	}
}
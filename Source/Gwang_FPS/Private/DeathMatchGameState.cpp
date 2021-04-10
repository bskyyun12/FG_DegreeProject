// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchGameState.h"
#include "Net/UnrealNetwork.h"

#include "DeathMatchGameMode.h"
#include "DeathMatchPlayerState.h"

void ADeathMatchGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADeathMatchGameState, MarvelTeamScore);
	DOREPLIFETIME(ADeathMatchGameState, DCTeamScore);
	DOREPLIFETIME(ADeathMatchGameState, MarvelPlayerStates);
	DOREPLIFETIME(ADeathMatchGameState, DCPlayerStates);
}

void ADeathMatchGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	UE_LOG(LogTemp, Warning, TEXT("(GameFlow) GameState::PostInitializeComponents (%s)"), *GetName());

	UFPSGameInstance* GI = GetGameInstance<UFPSGameInstance>();
	if (!ensure(GI != nullptr))
	{
		return;
	}
}

// Called by AGameModeBase::StartPlay()
void ADeathMatchGameState::HandleBeginPlay()
{
	Super::HandleBeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("(GameFlow) GameState::HandleBeginPlay (%s)"), *GetName());

	//GetDefaultGameMode<ADeathMatchGameMode>();
}

void ADeathMatchGameState::OnRep_ReplicatedHasBegunPlay()
{
	Super::OnRep_ReplicatedHasBegunPlay();
	UE_LOG(LogTemp, Warning, TEXT("(GameFlow) GameState::OnRep_ReplicatedHasBegunPlay (%s)"), *GetName());

}

void ADeathMatchGameState::AddScore(const ETeam& WinnerTeam)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (WinnerTeam == ETeam::Marvel)
		{
			MarvelTeamScore++;
		}
		else if (WinnerTeam == ETeam::DC)
		{
			DCTeamScore++;
		}
	}
	// TODO: Use OnRep_MarvelTeamScore to update client's ScoreBoardWidget
}

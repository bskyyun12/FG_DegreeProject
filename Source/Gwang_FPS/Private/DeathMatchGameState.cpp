// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchGameState.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

#include "DeathMatchGameMode.h"
#include "PlayerControllerInterface.h"
#include "DeathMatchPlayerState.h"

void ADeathMatchGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADeathMatchGameState, MarvelTeamScore);
	DOREPLIFETIME(ADeathMatchGameState, DCTeamScore);
}

void ADeathMatchGameState::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() == ROLE_Authority)
	{
		GM = Cast<ADeathMatchGameMode>(GetWorld()->GetAuthGameMode());
		if (!ensure(GM != nullptr))
		{
			return;
		}
		TimeLeftInSeconds = GM->GetMatchTimeInSeconds();
		ScoreToWin = GM->GetScoreToWin();

		UWorld* World = GetWorld();
		if (World != nullptr)
		{
			World->GetTimerManager().SetTimer(MatchTimer, this, &ADeathMatchGameState::MatchTimeCountdown, 1.f, true);
		}
	}
}

void ADeathMatchGameState::MatchTimeCountdown()
{
	TimeLeftInSeconds -= 1;
	if (TimeLeftInSeconds == 0)
	{
		EndMatch();
		GetWorld()->GetTimerManager().ClearTimer(MatchTimer);
	}

	for (APlayerState* PS : PlayerArray)
	{
		if (PS != nullptr)
		{
			APawn* Pawn = PS->GetPawn();
			if (Pawn != nullptr)
			{
				AController* Controller = Pawn->GetController();
				if (Controller != nullptr && UKismetSystemLibrary::DoesImplementInterface(Controller, UPlayerControllerInterface::StaticClass()))
				{
					IPlayerControllerInterface::Execute_UpdateMatchTimeUI(Controller, TimeLeftInSeconds);
				}
			}
		}
	}
}

void ADeathMatchGameState::AddScore(const ETeam& TeamToAddScore)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (TeamToAddScore == ETeam::Marvel)
		{
			MarvelTeamScore++;
		}
		else if (TeamToAddScore == ETeam::DC)
		{
			DCTeamScore++;
		}

		for (APlayerState* PS : PlayerArray)
		{
			if (PS != nullptr)
			{
				APawn* Pawn = PS->GetPawn();
				if (Pawn != nullptr)
				{
					AController* Controller = Pawn->GetController();
					if (Controller != nullptr && UKismetSystemLibrary::DoesImplementInterface(Controller, UPlayerControllerInterface::StaticClass()))
					{
						IPlayerControllerInterface::Execute_UpdateTeamScoreUI(Controller, MarvelTeamScore, DCTeamScore);
					}
				}
			}
		}

		if (MarvelTeamScore >= ScoreToWin || DCTeamScore >= ScoreToWin)
		{
			EndMatch();
		}
	}
}

FScoreboardData ADeathMatchGameState::GetScoreboardData(ADeathMatchPlayerState* PS)
{
	if (PS == nullptr)
	{
		return FScoreboardData();
	}

	for (const FScoreboardData& Data : ScoreboardData)
	{
		if (Data.PlayerId == PS->GetPlayerId())
		{
			return Data;
		}
	}

	ScoreboardData.Add(FScoreboardData(PS->GetPlayerId(), *PS->GetPlayerName(), PS->GetTeam()));
	return GetScoreboardData(PS);
}

void ADeathMatchGameState::SetScoreboardData(const FScoreboardData& NewData)
{
	for (FScoreboardData& Data : ScoreboardData)
	{
		if (Data.PlayerId == NewData.PlayerId)
		{
			Data = NewData;
		}
	}

	UpdateScoreBoard();
}

void ADeathMatchGameState::UpdateScoreBoard()
{
	for (APlayerState* PS : PlayerArray)
	{
		if (PS != nullptr)
		{
			APawn* Pawn = PS->GetPawn();
			if (Pawn != nullptr)
			{
				AController* Controller = Pawn->GetController();
				if (Controller != nullptr && UKismetSystemLibrary::DoesImplementInterface(Controller, UPlayerControllerInterface::StaticClass()))
				{
					IPlayerControllerInterface::Execute_UpdateScoreBoard(Controller, ScoreboardData);
				}
			}
		}
	}
}

void ADeathMatchGameState::EndMatch()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (MarvelTeamScore > DCTeamScore)
		{
			GM->EndMatch(ETeam::Marvel);
		}
		else if (DCTeamScore > MarvelTeamScore)
		{
			GM->EndMatch(ETeam::DC);
		}
		else if (MarvelTeamScore == DCTeamScore)
		{
			GM->EndMatch(ETeam::None, true);
		}
	}
}


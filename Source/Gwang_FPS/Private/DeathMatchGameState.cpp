// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchGameState.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

#include "DeathMatchGameMode.h"
#include "PlayerControllerInterface.h"

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
	UE_LOG(LogTemp, Warning, TEXT("TimeLeftInSeconds: (%i)"), TimeLeftInSeconds);
	if (TimeLeftInSeconds == 0)
	{
		GM->EndMatch();
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
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
	DOREPLIFETIME(ADeathMatchGameState, TimeLeftInSeconds);
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

	for (APlayerState* PS : PlayerArray)
	{
		UE_LOG(LogTemp, Warning, TEXT("PS->GetInstigatorController()->GetName(): (%s)"), *PS->GetInstigatorController()->GetName());
	}
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
	TimeLeftInSeconds -= 1.f;
	UE_LOG(LogTemp, Warning, TEXT("TimeLeftInSeconds: (%f)"), TimeLeftInSeconds);
	if (TimeLeftInSeconds <= 0.f)
	{
		GM->EndMatch();
		GetWorld()->GetTimerManager().ClearTimer(MatchTimer);
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
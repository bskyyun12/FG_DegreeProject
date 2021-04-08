// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSPlayerState.h"
#include "FPSPlayerController.h"

void AFPSPlayerState::Initialize(AFPSPlayerController* PC)
{
	PlayerController = Cast<AFPSPlayerController>(PC);

	PlayerController->OnKill.AddDynamic(this, &AFPSPlayerState::OnKill);
	PlayerController->OnDeath.AddDynamic(this, &AFPSPlayerState::OnDeath);

	UserScore.UserName = *GetPlayerName();

	UE_LOG(LogTemp, Warning, TEXT("Initialize (%s)"), *UserScore.UserName.ToString());
}

void AFPSPlayerState::OnKill()
{
	UserScore.Kills++;
	UE_LOG(LogTemp, Warning, TEXT("(%s) : Kills: %i"), *UserScore.UserName.ToString(), UserScore.Kills);
}

void AFPSPlayerState::OnDeath()
{
	UserScore.Deaths++;
	UE_LOG(LogTemp, Warning, TEXT("(%s) : Deaths: %i"), *UserScore.UserName.ToString(), UserScore.Deaths);
}

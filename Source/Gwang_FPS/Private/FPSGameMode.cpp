// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSGameMode.h"
#include "Kismet/GameplayStatics.h" // GetAllActorsOfClass
#include "Kismet/KismetSystemLibrary.h" // DoesImplementInterface

#include "FPSCharacter.h"
#include "FPSPlayerController.h"
#include "FPSPlayerControllerInterface.h"
#include "FPSPlayerStart.h"

void AFPSGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!ensure(NewPlayer != nullptr))
	{
		return;
	}

	PlayerControllers.Add(NewPlayer);
	if (UKismetSystemLibrary::DoesImplementInterface(NewPlayer, UFPSPlayerControllerInterface::StaticClass()))
	{
		IFPSPlayerControllerInterface::Execute_LoadTeamSelection(NewPlayer);
	}
}

void AFPSGameMode::BeginPlay()
{
	Super::BeginPlay();

	//TODO: feels horrible here... please find another way to implement this without using GetAllActorsOfClass() crap
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFPSPlayerStart::StaticClass(), PlayerStarts);
	for (int16 i = 0; i < PlayerStarts.Num(); i++)
	{
		AFPSPlayerStart* PlayerStart = Cast<AFPSPlayerStart>(PlayerStarts[i]);
		if (PlayerStart != nullptr)
		{
			if (PlayerStart->Team == ETeam::Marvel)
			{
				MarvelTeamSpawnTransforms.Add(PlayerStart->GetActorTransform());
			}
			else if (PlayerStart->Team == ETeam::DC)
			{
				DCTeamSpawnTransforms.Add(PlayerStart->GetActorTransform());
			}
		}
	}

	if (!ensure(MarvelTeamCharacter != nullptr) || !ensure(DCTeamCharacter != nullptr))
	{
		return;
	}
}

void AFPSGameMode::SpawnPlayer(APlayerController* PlayerController, ETeam Team)
{
	if (PlayerController != nullptr && UKismetSystemLibrary::DoesImplementInterface(PlayerController, UFPSPlayerControllerInterface::StaticClass()))
	{
		if (Team == ETeam::Marvel)
		{
			IFPSPlayerControllerInterface::Execute_OnSpawnPlayer(PlayerController, MarvelTeamCharacter);
		}
		else if (Team == ETeam::DC)
		{
			IFPSPlayerControllerInterface::Execute_OnSpawnPlayer(PlayerController, DCTeamCharacter);
		}
	}
}

FTransform AFPSGameMode::GetRandomPlayerStarts(ETeam Team)
{
	FTransform output;

	if (Team == ETeam::Marvel)
	{
		int16 RandomIndex = FMath::RandRange(0, MarvelTeamSpawnTransforms.Num() - 1);
		output = MarvelTeamSpawnTransforms[RandomIndex];
	}
	else if (Team == ETeam::DC)
	{
		int16 RandomIndex = FMath::RandRange(0, DCTeamSpawnTransforms.Num() - 1);
		output = DCTeamSpawnTransforms[RandomIndex];
	}

	return output;
}

void AFPSGameMode::OnPlayerDeath(APlayerController* PlayerController, ETeam Team)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSGameMode::OnPlayerDeath()"));

	if (Team == ETeam::Marvel)
	{
		CurrentDCScore++;
	}
	else if (Team == ETeam::DC)
	{
		CurrentMarvelScore++;
	}	

	CheckGameOver(PlayerController);
}

void AFPSGameMode::CheckGameOver(APlayerController* PlayerController)
{
	bool bIsGameOver = false;
	if (CurrentDCScore >= KillScoreToWin)
	{
		UE_LOG(LogTemp, Warning, TEXT("DC Team Won!!!"));
		bIsGameOver = true;
	}
	else if (CurrentMarvelScore >= KillScoreToWin)
	{
		UE_LOG(LogTemp, Warning, TEXT("Marvel Team Won!!!"));
		bIsGameOver = true;
	}

	if (bIsGameOver)
	{
		for (int i = 0; i < PlayerControllers.Num(); i++)
		{
			IFPSPlayerControllerInterface::Execute_LoadGameOver(PlayerControllers[i]);
		}
	}
}

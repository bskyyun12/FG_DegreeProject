// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSGameMode.h"
#include "Kismet/GameplayStatics.h" // GetAllActorsOfClass
#include "Kismet/KismetSystemLibrary.h" // DoesImplementInterface

#include "FPSCharacter.h"
#include "FPSGameStateBase.h"
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

	if (UKismetSystemLibrary::DoesImplementInterface(NewPlayer, UFPSPlayerControllerInterface::StaticClass()))
	{
		IFPSPlayerControllerInterface::Execute_StartNewGame(NewPlayer);
	}
}

void AFPSGameMode::StartNewGame(APlayerController* PlayerController)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSGameMode::StartNewGame()"));
	FreePlayer(PlayerController);

	OnStartNewGame.Broadcast();
}

void AFPSGameMode::BeginPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSGameMode::BeginPlay()"));
	Super::BeginPlay();

	FPSGameState = GetGameState<AFPSGameStateBase>();
	if (!ensure(FPSGameState != nullptr))
	{
		return;
	}
	FPSGameState->Initialize(this);

	SetupPlayerStarts();
	SetupPlayerPool();
}

bool AFPSGameMode::SpawnPlayer(APlayerController* PlayerController, ETeam Team)
{
	AFPSCharacter* PooledPlayer = PoolPlayer(Team);
	if (PooledPlayer == nullptr)
	{
		return false;
	}

	if (PlayerController != nullptr && UKismetSystemLibrary::DoesImplementInterface(PlayerController, UFPSPlayerControllerInterface::StaticClass()))
	{
		if (Team == ETeam::Marvel)
		{
			IFPSPlayerControllerInterface::Execute_OnSpawnPlayer(PlayerController, PooledPlayer);
		}
		else if (Team == ETeam::DC)
		{
			IFPSPlayerControllerInterface::Execute_OnSpawnPlayer(PlayerController, PooledPlayer);
		}
	}

	OnUpdateTeamSelectionUI.Broadcast(Team, CanJoin(Team));

	return true;
}

AFPSCharacter* AFPSGameMode::PoolPlayer(ETeam Team)
{
	if (Team == ETeam::Marvel)
	{
		for (int i = 0; i < MarvelTeamPlayers.Num(); i++)
		{
			if (MarvelTeamPlayers[i] != nullptr && MarvelTeamPlayers[i]->GetController() == nullptr)
			{
				MarvelTeamPlayers[i]->SetActorHiddenInGame(false);
				return MarvelTeamPlayers[i];
			}
		}
	}
	else if (Team == ETeam::DC)
	{
		for (int i = 0; i < DCTeamPlayers.Num(); i++)
		{
			if (DCTeamPlayers[i] != nullptr && DCTeamPlayers[i]->GetController() == nullptr)
			{
				DCTeamPlayers[i]->SetActorHiddenInGame(false);
				return DCTeamPlayers[i];
			}
		}
	}

	return nullptr;
}

bool AFPSGameMode::CanJoin(ETeam Team)
{
	if (Team == ETeam::Marvel)
	{
		for (int i = 0; i < MarvelTeamPlayers.Num(); i++)
		{
			if (MarvelTeamPlayers[i] != nullptr && MarvelTeamPlayers[i]->GetController() == nullptr)
			{
				return true;
			}
		}
	}
	else if (Team == ETeam::DC)
	{
		for (int i = 0; i < DCTeamPlayers.Num(); i++)
		{
			if (DCTeamPlayers[i] != nullptr && DCTeamPlayers[i]->GetController() == nullptr)
			{
				return true;
			}
		}
	}

	return false;
}

void AFPSGameMode::FreePlayer(APlayerController* PlayerController)
{
	if (PlayerController != nullptr)
	{
		if (PlayerController->GetPawn() != nullptr)
		{
			PlayerController->GetPawn()->SetActorHiddenInGame(true);
		}
		PlayerController->UnPossess();
	}
}

FTransform AFPSGameMode::GetRandomPlayerStarts(ETeam Team)
{
	TArray<FTransform> SpawnTransforms = (Team == ETeam::Marvel) ? MarvelTeamSpawnTransforms : DCTeamSpawnTransforms;
	int16 RandomIndex = FMath::RandRange(0, SpawnTransforms.Num() - 1);

	return SpawnTransforms[RandomIndex];
}

void AFPSGameMode::OnPlayerDeath(APlayerController* PlayerController, ETeam Team)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSGameMode::OnPlayerDeath()"));

	FPSGameState->OnPlayerDeath(Team);
}

void AFPSGameMode::CheckGameOver(int MarvelScore, int DCScore)
{
	bool MarvelWon = MarvelScore >= KillScoreToWin;
	bool DCWon = DCScore >= KillScoreToWin;
	if (MarvelWon)
	{
		OnEndGame.Broadcast(ETeam::Marvel);
	}
	else
	{
		OnEndGame.Broadcast(ETeam::DC);
	}
}

void AFPSGameMode::SetupPlayerPool()
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	if (!ensure(MarvelTeamCharacter != nullptr) || !ensure(DCTeamCharacter != nullptr))
	{
		return;
	}

	// players to pool
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	for (int i = 0; i < MaxPlayerPerTeam; i++)
	{
		AFPSCharacter* SpawnedCharacter = World->SpawnActor<AFPSCharacter>(MarvelTeamCharacter, GetRandomPlayerStarts(ETeam::Marvel), SpawnParams);
		SpawnedCharacter->SetActorHiddenInGame(true);
		MarvelTeamPlayers.Add(SpawnedCharacter);
	}

	for (int i = 0; i < MaxPlayerPerTeam; i++)
	{
		AFPSCharacter* SpawnedCharacter = World->SpawnActor<AFPSCharacter>(DCTeamCharacter, GetRandomPlayerStarts(ETeam::DC), SpawnParams);
		SpawnedCharacter->SetActorHiddenInGame(true);
		DCTeamPlayers.Add(SpawnedCharacter);
	}
}

void AFPSGameMode::SetupPlayerStarts()
{
	// Get all player starts
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
}
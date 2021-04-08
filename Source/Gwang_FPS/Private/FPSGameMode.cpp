// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSGameMode.h"
#include "Kismet/GameplayStatics.h" // GetAllActorsOfClass
#include "Kismet/KismetSystemLibrary.h" // DoesImplementInterface

#include "FPSCharacter.h"
#include "FPSGameState.h"
#include "FPSPlayerController.h"
#include "FPSPlayerControllerInterface.h"
#include "FPSPlayerStart.h"

void AFPSGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	UE_LOG(LogTemp, Warning, TEXT("AFPSGameMode::InitGame()"));

	SetupPlayerStarts();
	SetupPlayerPool();
}

void AFPSGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	UE_LOG(LogTemp, Warning, TEXT("AFPSGameMode::PostLogin()"));

	if (NewPlayer != nullptr && UKismetSystemLibrary::DoesImplementInterface(NewPlayer, UFPSPlayerControllerInterface::StaticClass()))
	{
		IFPSPlayerControllerInterface::Execute_OnPostLogin(NewPlayer, this);
	}

	FPSGameState = GetGameState<AFPSGameState>();
	if (!ensure(GameState != nullptr))
	{
		return;
	}

}

void AFPSGameMode::BeginPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSGameMode::BeginPlay()"));
	Super::BeginPlay();
	FPSGameState = GetGameState<AFPSGameState>();
	if (!ensure(GameState != nullptr))
	{
		return;
	}
	FPSGameState->Init(this);

	StartGame();
}

void AFPSGameMode::StartGame()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSGameMode::StartGame()"));

	OnStartGame.Broadcast();
}

void AFPSGameMode::StartMatch()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSGameMode::StartMatch()"));

	OnStartMatch.Broadcast();
}

void AFPSGameMode::SpawnPlayer(APlayerController* PlayerController, ETeam Team)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSGameMode::SpawnPlayer()"));

	// 1. Free the incoming player controller's pawn
	FreePlayer(PlayerController->GetPawn());

	// 2. Try to pool a player in given team
	AFPSCharacter* SpawnedPlayer = PoolPlayer(Team);
	if (!ensure(SpawnedPlayer != nullptr))
	{
		return;
	}

	// 3. Spawn the pooled player
	if (PlayerController != nullptr && UKismetSystemLibrary::DoesImplementInterface(PlayerController, UFPSPlayerControllerInterface::StaticClass()))
	{
		IFPSPlayerControllerInterface::Execute_OnSpawnPlayer(PlayerController, SpawnedPlayer);
	}
}

AFPSCharacter* AFPSGameMode::PoolPlayer(ETeam Team)
{
	AFPSCharacter* PlayerToPool = nullptr;

	if (Team == ETeam::Marvel)
	{
		for (int i = 0; i < MarvelPlayers.Num(); i++)
		{
			if (MarvelPlayers[i] != nullptr && MarvelPlayers[i]->GetController() == nullptr)
			{
				PlayerToPool = MarvelPlayers[i];
				break;
			}
		}
	}
	else if (Team == ETeam::DC)
	{
		for (int i = 0; i < DCPlayers.Num(); i++)
		{
			if (DCPlayers[i] != nullptr && DCPlayers[i]->GetController() == nullptr)
			{
				PlayerToPool = DCPlayers[i];
				break;
			}
		}
	}

	if (PlayerToPool != nullptr)
	{
		PlayerToPool->SetActorHiddenInGame(false);
		PlayerToPool->SetActorEnableCollision(true);
	}

	return PlayerToPool;
}

void AFPSGameMode::FreePlayer(APawn* Player)
{
	if (Player != nullptr)
	{
		Player->SetActorHiddenInGame(true);
		Player->SetActorEnableCollision(false);

		if (Player->GetController() != nullptr)
		{
			Player->GetController()->UnPossess();
		}
	}
}

FTransform AFPSGameMode::GetRandomPlayerStarts(ETeam Team)
{
	TArray<FTransform> SpawnTransforms = (Team == ETeam::Marvel) ? MarvelTeamSpawnTransforms : DCTeamSpawnTransforms;

	if (SpawnTransforms.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("AFPSGameMode::GetRandomPlayerStarts()"));
		UE_LOG(LogTemp, Warning, TEXT("SpawnTransforms.Num() == 0"));
		return FTransform();
	}
	int16 RandomIndex = FMath::RandRange(0, SpawnTransforms.Num() - 1);

	return SpawnTransforms[RandomIndex];
}

void AFPSGameMode::OnPlayerDeath(APlayerController* PlayerController, ETeam Team)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSGameMode::OnPlayerDeath()"));
	if (PlayerController != nullptr)
	{
		FreePlayer(PlayerController->GetPawn());
	}

	ETeam WinnerTeam = GetWinnerTeam();
	if (WinnerTeam != ETeam::None)
	{
		EndMatch(WinnerTeam);
	}
}

void AFPSGameMode::EndMatch(ETeam WinnerTeam)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSGameState::EndMatch()"));
	
	OnEndMatch.Broadcast();

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	FTimerHandle MatchStartTimer;
	World->GetTimerManager().SetTimer(MatchStartTimer, [&]()
	{
		StartMatch();
	}, 5.f, false);
}

ETeam AFPSGameMode::GetWinnerTeam()
{
	// TODO: implement this again without using loops?
	int MarvelSurvivors = 0;
	int DCSurvivors = 0;

	for (AFPSCharacter* MarvelPlayer : MarvelPlayers)
	{
		if (MarvelPlayer->GetController() != nullptr)
		{
			MarvelSurvivors++;
		}
	}

	for (AFPSCharacter* DCPlayer : DCPlayers)
	{
		if (DCPlayer->GetController() != nullptr)
		{
			DCSurvivors++;
		}
	}

	if (MarvelSurvivors == 0)
	{
		return ETeam::DC;
	}
	else if (DCSurvivors == 0)
	{
		return ETeam::Marvel;
	}
	else
	{
		return ETeam::None;
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
	for (int i = 0; i < MaxPlayersPerTeam; i++)
	{
		AFPSCharacter* SpawnedCharacter = World->SpawnActor<AFPSCharacter>(MarvelTeamCharacter, GetRandomPlayerStarts(ETeam::Marvel), SpawnParams);
		MarvelPlayers.Add(SpawnedCharacter);
		FreePlayer(SpawnedCharacter);
	}

	for (int i = 0; i < MaxPlayersPerTeam; i++)
	{
		AFPSCharacter* SpawnedCharacter = World->SpawnActor<AFPSCharacter>(DCTeamCharacter, GetRandomPlayerStarts(ETeam::DC), SpawnParams);
		DCPlayers.Add(SpawnedCharacter);
		FreePlayer(SpawnedCharacter);
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

/////////////////
// Doing this because I want to be able to play from FPS_Gwang map through engine!
ETeam AFPSGameMode::GetTeamWithLessPeople()
{
	int MarvelTeamCounter = 0;
	int DCTeamCounter = 0;
	for (AFPSCharacter* MarvelPlayer : MarvelPlayers)
	{
		if (MarvelPlayer->GetController() != nullptr)
		{
			MarvelTeamCounter++;
		}
	}
	for (AFPSCharacter* DCPlayer : DCPlayers)
	{
		if (DCPlayer->GetController() != nullptr)
		{
			DCTeamCounter++;
		}
	}
	return MarvelTeamCounter <= DCTeamCounter ? ETeam::Marvel : ETeam::DC;
}
/////////////////
// Doing this because I want to be able to play from FPS_Gwang map through engine!
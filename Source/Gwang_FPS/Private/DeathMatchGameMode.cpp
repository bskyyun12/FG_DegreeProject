// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchGameMode.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"	// TActorIterator
#include "GameFramework/PlayerStart.h"
#include "Kismet/KismetSystemLibrary.h"

#include "DeathMatchPlayerController.h"
#include "DeathMatchCharacter.h"
#include "DeathMatchPlayerState.h"
#include "DeathMatchGameState.h"
#include "PlayerControllerInterface.h"

void ADeathMatchGameMode::InitGameState()
{
	Super::InitGameState();

	GS = GetGameState<ADeathMatchGameState>();
	if (!ensure(GS != nullptr))
	{
		return;
	}
}

// Called after a successful login
void ADeathMatchGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ADeathMatchPlayerController* PC = Cast<ADeathMatchPlayerController>(NewPlayer);
	if (!ensure(PC != nullptr))
	{
		return;
	}

	PlayerControllers.Add(PC);
	PC->Server_OnPostLogin();

	ADeathMatchPlayerState* PS = PC->GetPlayerState<ADeathMatchPlayerState>();
	if (!ensure(PS != nullptr))
	{
		return;
	}
	PS->Server_OnPostLogin();
}

// Called when a player leaves the game or is destroyed.
void ADeathMatchGameMode::Logout(AController* Exiting)
{
	//UE_LOG(LogTemp, Warning, TEXT("(GameFlow) GameMode::Logout => ( %s ) Logged out"), *Exiting->GetName());

	PlayerControllers.Remove(Cast<ADeathMatchPlayerController>(Exiting));

	Super::Logout(Exiting);
}

// Spawn players and start the game!
void ADeathMatchGameMode::RestartMatch()
{
	for (ADeathMatchPlayerController* PC : PlayerControllers)
	{
		SpawnPlayer(PC);
	}
}

void ADeathMatchGameMode::SpawnPlayer(ADeathMatchPlayerController* PC)
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	if (PC != nullptr)
	{
		ADeathMatchPlayerState* PS = PC->GetPlayerState<ADeathMatchPlayerState>();
		if (PS != nullptr)
		{
			ETeam Team = PS->GetTeam();

			// Team is None if the player did not choose a team from a lobby
			if (Team == ETeam::None)
			{
				// Assign a team with less players
				Team = GetTeamWithLessPeople();
				PS->Server_SetTeam(Team);
			}

			const TSubclassOf<ADeathMatchCharacter> CharacterClass = (Team == ETeam::Marvel) ? Marvel_CharacterClass : DC_CharacterClass;
			const FString PlayerStartTag = (Team == ETeam::Marvel) ? "Marvel" : "DC";
			const AActor* PlayerStart = GetBestPlayerStart(PlayerStartTag);
			if (PlayerStart == nullptr)
			{
				PlayerStart = FindPlayerStart(PC, PlayerStartTag);
			}

			if (PC->GetPawn() != nullptr)
			{
				PC->GetPawn()->Destroy();
			}

			ADeathMatchCharacter* SpawnedPlayer = World->SpawnActor<ADeathMatchCharacter>(CharacterClass, PlayerStart->GetActorTransform());
			PC->Possess(SpawnedPlayer);
			PC->ClientSetRotation(PlayerStart->GetActorRotation(), true);

			PS->Server_OnSpawn();
			PC->Server_OnSpawnPlayer(SpawnedPlayer);
		}
	}
}

AActor* ADeathMatchGameMode::GetBestPlayerStart(const FString& PlayerStartTag) const
{
	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			APlayerStart* PlayerStart = *It;
			if (PlayerStart && PlayerStart->PlayerStartTag == *PlayerStartTag)
			{
				FHitResult Hit;
				bool bHit = World->LineTraceSingleByChannel(Hit, PlayerStart->GetActorLocation(), PlayerStart->GetActorLocation() + FVector::ForwardVector, ECC_Visibility);
				if (!bHit)
				{
					return PlayerStart;
				}
			}
		}
	}

	return nullptr;
}

// Called after ADeathMatchPlayerController::Server_OnPlayerDeath
void ADeathMatchGameMode::OnPlayerDeath(ADeathMatchPlayerController* PC)
{
	// Respawn player
	FTimerHandle RespawnTimer;
	FTimerDelegate RespawnTimerDel;
	RespawnTimerDel.BindUObject(this, &ADeathMatchGameMode::SpawnPlayer, PC);
	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		World->GetTimerManager().SetTimer(RespawnTimer, RespawnTimerDel, RespawnDelay, false);
	}
}

// Finish the game and move players to lobby
void ADeathMatchGameMode::EndMatch(const ETeam& WinnerTeam, const bool& bIsDraw)
{
	for (ADeathMatchPlayerController* PC : PlayerControllers)
	{
		if (PC != nullptr && UKismetSystemLibrary::DoesImplementInterface(PC, UPlayerControllerInterface::StaticClass()))
		{
			IPlayerControllerInterface::Execute_LoadGameOverUI(PC, WinnerTeam, bIsDraw);
		}
	}

	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		World->ServerTravel("/Game/Maps/Lobby?listen");
	}
}

ETeam ADeathMatchGameMode::GetTeamWithLessPeople()
{
	int MarvelTeamCounter = 0;
	int DCTeamCounter = 0;
	for (APlayerState* PlayerState : GS->PlayerArray)
	{
		ADeathMatchPlayerState* PS = Cast<ADeathMatchPlayerState>(PlayerState);
		if (PS != nullptr)
		{
			if (PS->GetTeam() == ETeam::Marvel)
			{
				MarvelTeamCounter++;
			}
			else if (PS->GetTeam() == ETeam::DC)
			{
				DCTeamCounter++;
			}
		}
	}
	return MarvelTeamCounter <= DCTeamCounter ? ETeam::Marvel : ETeam::DC;
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchGameMode.h"
#include "GameFramework/PlayerStart.h"

#include "DeathMatchPlayerController.h"
#include "DeathMatchCharacter.h"
#include "DeathMatchPlayerState.h"
#include "DeathMatchGameState.h"
#include <DrawDebugHelpers.h>
#include <EngineUtils.h>

// Called after a successful login
void ADeathMatchGameMode::PostLogin(APlayerController* NewPlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("(GameFlow) ----------------------------------------------------"));
	UE_LOG(LogTemp, Warning, TEXT("(GameFlow) GameMode::PostLogin => ( %s ) successfully Logged in"), *NewPlayer->GetName());

	Super::PostLogin(NewPlayer);
	PlayerControllers.Add(Cast<ADeathMatchPlayerController>(NewPlayer));
}

// Called when a player leaves the game or is destroyed.
void ADeathMatchGameMode::Logout(AController* Exiting)
{
	UE_LOG(LogTemp, Warning, TEXT("(GameFlow) GameMode::Logout => ( %s ) Logged out"), *Exiting->GetName());
	Super::Logout(Exiting);
	PlayerControllers.Remove(Cast<ADeathMatchPlayerController>(Exiting));
}

void ADeathMatchGameMode::BeginPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("(GameFlow) GameMode::BeginPlay"));
	Super::BeginPlay();

	// TEMP
	FTimerHandle StartTimer;
	GetWorld()->GetTimerManager().SetTimer(StartTimer, [&]()
		{
			for (ADeathMatchPlayerController* PC : PlayerControllers)
			{
				SpawnPlayers();
			}
		}, 3.f, false);
}

void ADeathMatchGameMode::SpawnPlayers()
{
	UE_LOG(LogTemp, Warning, TEXT("(GameFlow) GameMode::SpawnPlayers"));
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	for (ADeathMatchPlayerController* PC : PlayerControllers)
	{
		if (PC != nullptr)
		{
			ADeathMatchPlayerState* PS = PC->GetPlayerState<ADeathMatchPlayerState>();
			if (PS != nullptr)
			{
				ETeam Team = PS->GetPlayerInfo().Team;

				// Team is None if the player did not choose a team from lobby
				if (Team == ETeam::None)
				{
					// Assign a team with less players
					Team = GetTeamWithLessPlayers();
					PS->Server_SetTeam(Team);
				}

				const TSubclassOf<ADeathMatchCharacter> CharacterClass = (Team == ETeam::Marvel) ? Marvel_CharacterClass : DC_CharacterClass;
				const FString PlayerStartTag = (Team == ETeam::Marvel) ? "Marvel" : "DC";
				const AActor* PlayerStart = GetBestPlayerStart(PlayerStartTag);
				if (PlayerStart == nullptr)
				{
					PlayerStart = FindPlayerStart(PC, PlayerStartTag);
				}

				if (PC->GetPawn() == nullptr)
				{
					//ADeathMatchCharacter* SpawnedCharacter = World->SpawnActor<ADeathMatchCharacter>(CharacterClass, SpawnLocation, PlayerStart->GetActorRotation());
					ADeathMatchCharacter* SpawnedCharacter = World->SpawnActor<ADeathMatchCharacter>(CharacterClass, PlayerStart->GetActorTransform());
					PC->Possess(SpawnedCharacter);
				}
				else
				{
					//PC->GetPawn()->SetActorLocation(SpawnLocation);
					PC->GetPawn()->SetActorLocation(PlayerStart->GetActorLocation());
					PC->ClientSetRotation(PlayerStart->GetActorRotation(), true);
				}

				//UE_LOG(LogTemp, Warning, TEXT("(GameFlow) GameMode::SpawnPlayers => ( %s ) Spawned! Team: %i"), *PC->GetPawn()->GetName(), Team);
			}
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

ETeam ADeathMatchGameMode::GetTeamWithLessPlayers()
{
	int MarvelTeamCounter = 0;
	int DCTeamCounter = 0;
	ADeathMatchGameState* GS = GetGameState<ADeathMatchGameState>();
	if (GS != nullptr)
	{
		for (APlayerState* PlayerState : GS->PlayerArray)
		{
			ADeathMatchPlayerState* PS = Cast<ADeathMatchPlayerState>(PlayerState);
			if (PS != nullptr)
			{
				if (PS->GetPlayerInfo().Team == ETeam::Marvel)
				{
					MarvelTeamCounter++;
				}
				else if (PS->GetPlayerInfo().Team == ETeam::DC)
				{
					DCTeamCounter++;
				}
			}
		}
	}
	return MarvelTeamCounter <= DCTeamCounter ? ETeam::Marvel : ETeam::DC;
}

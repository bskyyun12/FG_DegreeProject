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
	Super::PostLogin(NewPlayer);
	UE_LOG(LogTemp, Warning, TEXT("(GameFlow) ----------------------------------------------------"));
	UE_LOG(LogTemp, Warning, TEXT("(GameFlow) GameMode::PostLogin => ( %s ) successfully Logged in"), *NewPlayer->GetName());

	ADeathMatchPlayerController* PC = Cast<ADeathMatchPlayerController>(NewPlayer);
	PC->OnPostLogin();

	PlayerControllers.Add(PC);
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

	GS = GetGameState<ADeathMatchGameState>();
	if (!ensure(GS != nullptr))
	{
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(MatchStartTimer, this, &ADeathMatchGameMode::MatchStartCheck, 3.f, true);
}

void ADeathMatchGameMode::MatchStartCheck()
{
	UE_LOG(LogTemp, Warning, TEXT("GameMode::MatchStartCheck => GS->PlayerArray.Num(): %i"), GS->PlayerArray.Num());

	StartMatch();


	// TODO: check if all player are joined the game?
	int NumPlayers = 3;

	if (NumPlayers == GS->PlayerArray.Num())
	{
		//StartMatch();
		//GetWorld()->GetTimerManager().ClearTimer(MatchStartTimer);
	}
}

// Spawn players and start the game!
void ADeathMatchGameMode::StartMatch()
{
	UE_LOG(LogTemp, Warning, TEXT("(GameFlow) GameMode::StartMatch"));

	for (ADeathMatchPlayerController* PC : PlayerControllers)
	{
		SpawnPlayer(PC);
	}

	OnStartMatch.Broadcast();
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

			// Team is None if the player did not choose a team from lobby
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

			if (PC->GetPawn() == nullptr)
			{
				ADeathMatchCharacter* SpawnedCharacter = World->SpawnActor<ADeathMatchCharacter>(CharacterClass, PlayerStart->GetActorTransform());
				PC->Possess(SpawnedCharacter);
			}
			else
			{
				PC->GetPawn()->SetActorLocation(PlayerStart->GetActorLocation());
				PC->ClientSetRotation(PlayerStart->GetActorRotation(), true);
			}

			//UE_LOG(LogTemp, Warning, TEXT("(GameFlow) GameMode::SpawnPlayers => ( %s ) Spawned! Team: %i"), *PC->GetPawn()->GetName(), Team);
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

// Finish the game and move players to lobby
void ADeathMatchGameMode::EndMatch()
{
	UE_LOG(LogTemp, Warning, TEXT("(GameFlow) GameMode::EndMatch"));
	// TODO: Server travel to lobby

	OnEndMatch.Broadcast();
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

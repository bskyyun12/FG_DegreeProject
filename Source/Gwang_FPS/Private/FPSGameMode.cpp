// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSGameMode.h"
#include "Kismet/GameplayStatics.h" // GetAllActorsOfClass
#include "Kismet/KismetSystemLibrary.h" // DoesImplementInterface

#include "FPSPlayerController.h"
#include "FPSPlayerControllerInterface.h"
#include "FPSPlayerStart.h"

void AFPSGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// interface approach
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
			if (PlayerStart->Team == ETeam::Dark)
			{
				DarkCharacterSpawnTransforms.Add(PlayerStart->GetActorTransform());
			}
			else if (PlayerStart->Team == ETeam::Silver)
			{
				SilverCharacterSpawnTransforms.Add(PlayerStart->GetActorTransform());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Add the new team in the enum. enum location: FPSPlayerStart.h"));
				ensure(false);
			}
		}
	}
}

void AFPSGameMode::SpawnPlayer(APlayerController* PlayerController, ETeam Team)
{
	if (UKismetSystemLibrary::DoesImplementInterface(PlayerController, UFPSPlayerControllerInterface::StaticClass()))
	{
		if (Team == ETeam::Dark)
		{
			IFPSPlayerControllerInterface::Execute_OnSpawnPlayer(PlayerController, DarkCharacter);
		}
		else if (Team == ETeam::Silver)
		{
			IFPSPlayerControllerInterface::Execute_OnSpawnPlayer(PlayerController, SilverCharacter);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Add the new team in the enum. enum location: FPSPlayerStart.h"));
			ensure(false);
		}
	}
}

FTransform AFPSGameMode::GetRandomPlayerStarts(ETeam Team)
{
	FTransform output;

	if (Team == ETeam::Dark)
	{
		int16 RandomIndex = FMath::RandRange(0, DarkCharacterSpawnTransforms.Num() - 1);
		output = DarkCharacterSpawnTransforms[RandomIndex];
	}
	else if (Team == ETeam::Silver)
	{
		int16 RandomIndex = FMath::RandRange(0, SilverCharacterSpawnTransforms.Num() - 1);
		output = SilverCharacterSpawnTransforms[RandomIndex];
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Add the new team in the enum. enum location: FPSPlayerStart.h"));
		ensure(false);
	}

	return output;
}
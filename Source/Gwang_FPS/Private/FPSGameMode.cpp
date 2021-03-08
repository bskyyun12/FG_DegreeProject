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

	// casting approach
	//AFPSPlayerController* PlayerController = Cast<AFPSPlayerController>(NewPlayer);
	//if (!ensure(PlayerController != nullptr))
	//{
	//	return;
	//}
	//PlayerController->LoadTeamSelection();

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
			if (PlayerStart->bIsDarkTeamStart)
			{
				DarkCharacterSpawnTransforms.Add(PlayerStart->GetActorTransform());
			}
			else
			{
				SilverCharacterSpawnTransforms.Add(PlayerStart->GetActorTransform());
			}
		}
	}
}

void AFPSGameMode::SpawnDarkTeam(APlayerController* PlayerController)
{
	if (UKismetSystemLibrary::DoesImplementInterface(PlayerController, UFPSPlayerControllerInterface::StaticClass()))
	{
		IFPSPlayerControllerInterface::Execute_OnSpawnPlayer(PlayerController, DarkCharacter, true);
	}
}

void AFPSGameMode::SpawnSilverTeam(APlayerController* PlayerController)
{
	if (UKismetSystemLibrary::DoesImplementInterface(PlayerController, UFPSPlayerControllerInterface::StaticClass()))
	{
		IFPSPlayerControllerInterface::Execute_OnSpawnPlayer(PlayerController, SilverCharacter, false);
	}
}

FTransform AFPSGameMode::GetRandomPlayerStarts(bool bIsDarkTeam)
{
	if (bIsDarkTeam)
	{
		int16 RandomIndex = FMath::RandRange(0, DarkCharacterSpawnTransforms.Num() - 1);
		return DarkCharacterSpawnTransforms[RandomIndex];
	}
	else
	{
		int16 RandomIndex = FMath::RandRange(0, SilverCharacterSpawnTransforms.Num() - 1);
		return SilverCharacterSpawnTransforms[RandomIndex];
	}
}

void AFPSGameMode::RespawnPlayer(APlayerController* PlayerController)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSGameMode::RespawnPlayer"));
	if (UKismetSystemLibrary::DoesImplementInterface(PlayerController, UFPSPlayerControllerInterface::StaticClass()))
	{
		IFPSPlayerControllerInterface::Execute_OnRespawnPlayer(PlayerController);
	}
}

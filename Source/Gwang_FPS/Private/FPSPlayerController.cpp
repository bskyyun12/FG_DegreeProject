// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSPlayerController.h"
#include "Widgets/TeamSelectionWidget.h"
#include "Kismet/GameplayStatics.h"

#include "FPSGameMode.h"
#include "FPSCharacter.h"

AFPSPlayerController::AFPSPlayerController(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
{

}

void AFPSPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AFPSPlayerController::LoadTeamSelection_Implementation()
{
	Client_LoadTeamSelection(TeamSelectionClass);
}

void AFPSPlayerController::Client_LoadTeamSelection_Implementation(TSubclassOf<UUserWidget> teamSelectionClass)
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	TeamSelection = CreateWidget<UTeamSelectionWidget>(World, teamSelectionClass);
	if (!ensure(TeamSelection != nullptr))
	{
		return;
	}

	TeamSelection->Setup();
}

void AFPSPlayerController::OnDarkTeamSelected_Implementation()
{
	TeamSelection->Teardown();
	Server_OnDarkTeamSelected();
}

void AFPSPlayerController::Server_OnDarkTeamSelected_Implementation()
{
	if (GameMode == nullptr)
	{
		UWorld* World = GetWorld();
		if (!ensure(World != nullptr))
		{
			return;
		}
		GameMode = Cast<AFPSGameMode>(UGameplayStatics::GetGameMode(World));
		if (!ensure(GameMode != nullptr))
		{
			return;
		}
	}
	GameMode->SpawnDarkTeam(this);
}

void AFPSPlayerController::OnSilverTeamSelected_Implementation()
{
	TeamSelection->Teardown();
	Server_OnSilverTeamSelected();
}

void AFPSPlayerController::Server_OnSilverTeamSelected_Implementation()
{
	if (GameMode == nullptr)
	{
		UWorld* World = GetWorld();
		if (!ensure(World != nullptr))
		{
			return;
		}
		GameMode = Cast<AFPSGameMode>(UGameplayStatics::GetGameMode(World));
		if (!ensure(GameMode != nullptr))
		{
			return;
		}
	}
	GameMode->SpawnSilverTeam(this);
}

void AFPSPlayerController::OnSpawnPlayer_Implementation(TSubclassOf<AFPSCharacter> CharacterClass, bool bIsDarkTeam)
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	FTransform SpawnTransform = GameMode->GetRandomPlayerStarts(bIsDarkTeam);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AFPSCharacter* SpawnedCharacter = World->SpawnActor<AFPSCharacter>(CharacterClass, SpawnTransform, SpawnParams);
	this->Possess(SpawnedCharacter);
	SpawnedCharacter->bIsDarkTeam = bIsDarkTeam;
}

void AFPSPlayerController::RespawnPlayer_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::OnPlayerDeath_Implementation"));
	AFPSCharacter* FPSPlayer = Cast<AFPSCharacter>(GetPawn());
	if (FPSPlayer != nullptr)
	{
		FTransform SpawnTransform = GameMode->GetRandomPlayerStarts(FPSPlayer->bIsDarkTeam);
		FPSPlayer->SetActorTransform(SpawnTransform);
	}
}
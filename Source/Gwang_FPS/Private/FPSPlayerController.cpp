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

void AFPSPlayerController::OnTeamSelected_Implementation(ETeam InTeam)
{
	TeamSelection->Teardown();
	Server_OnTeamSelected(InTeam);
}

void AFPSPlayerController::Server_OnTeamSelected_Implementation(ETeam InTeam)
{
	Team = InTeam;
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
	GameMode->SpawnPlayer(this, Team);
}

void AFPSPlayerController::ShakeCamera_Implementation(TSubclassOf<UCameraShakeBase> CameraShake)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::ShakeCamera_Implementation"));
	if (CameraShake != nullptr)
	{
		ClientStartCameraShake(CameraShake);
	}
}

void AFPSPlayerController::OnSpawnPlayer_Implementation(TSubclassOf<AFPSCharacter> CharacterClass)
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	FTransform SpawnTransform = GameMode->GetRandomPlayerStarts(Team);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AFPSCharacter* SpawnedCharacter = World->SpawnActor<AFPSCharacter>(CharacterClass, SpawnTransform, SpawnParams);
	this->Possess(SpawnedCharacter);
	SpawnedCharacter->OnPossessed(this);;
}

void AFPSPlayerController::RespawnPlayer_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::OnPlayerDeath_Implementation"));

	FTransform SpawnTransform = GameMode->GetRandomPlayerStarts(Team);
	if (GetPawn() != nullptr)
	{
		GetPawn()->SetActorTransform(SpawnTransform);
	}
}
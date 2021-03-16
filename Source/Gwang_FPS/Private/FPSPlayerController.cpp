// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSPlayerController.h"
#include "Kismet/GameplayStatics.h"

#include "FPSCharacter.h"
#include "FPSGameMode.h"
#include "Widgets/TeamSelectionWidget.h"
#include "Widgets/GameOverWidget.h"

void AFPSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!ensure(TeamSelectionClass != nullptr))
	{
		return;
	}
	if (!ensure(GameOverWidgetClass != nullptr))
	{
		return;
	}
}

void AFPSPlayerController::StartNewGame_Implementation()
{
	Server_StartNewGame();
}

void AFPSPlayerController::Server_StartNewGame_Implementation()
{
	if (GameMode == nullptr)
	{
		UWorld* World = GetWorld();
		if (!ensure(World != nullptr))
		{
			return;
		}
		GameMode = Cast<AFPSGameMode>(UGameplayStatics::GetGameMode(World));
	}

	if (!ensure(GameMode != nullptr))
	{
		return;
	}

	GameMode->OnUpdateTeamSelectionUI.RemoveAll(this);
	GameMode->OnUpdateTeamSelectionUI.AddDynamic(this, &AFPSPlayerController::OnUpdateTeamSelectionUI);

	GameMode->StartNewGame(this);

	Client_LoadTeamSelection();
}

void AFPSPlayerController::OnUpdateTeamSelectionUI(ETeam InTeam, bool bCanJoinTeam)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::OnUpdateTeamSelectionUI()"));
	Client_OnUpdateTeamSelectionUI(InTeam, bCanJoinTeam);
}

void AFPSPlayerController::Client_OnUpdateTeamSelectionUI_Implementation(ETeam InTeam, bool bCanJoinTeam)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::Client_OnUpdateTeamSelectionUI_Implementation()"));
	if (TeamSelection != nullptr)
	{
		TeamSelection->OnTeamFilled(InTeam, bCanJoinTeam);
	}
}

void AFPSPlayerController::Client_LoadTeamSelection_Implementation()
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	TeamSelection = CreateWidget<UTeamSelectionWidget>(World, TeamSelectionClass);
	if (!ensure(TeamSelection != nullptr))
	{
		return;
	}

	TeamSelection->Setup();
}

void AFPSPlayerController::OnTeamSelected_Implementation(ETeam InTeam)
{
	if (TeamSelection != nullptr)
	{
		TeamSelection->Teardown();
		Server_OnTeamSelected(InTeam);
	}
}

void AFPSPlayerController::Server_OnTeamSelected_Implementation(ETeam InTeam)
{
	Team = InTeam;

	// Shouldn't fail to spawn but if does, start over
	if (GameMode->SpawnPlayer(this, Team) == false)
	{
		StartNewGame_Implementation();
	}
}

void AFPSPlayerController::OnSpawnPlayer_Implementation(AFPSCharacter* PooledPlayer)
{
	if (PooledPlayer != nullptr)
	{
		FTransform SpawnTransform = GameMode->GetRandomPlayerStarts(Team);
		PooledPlayer->SetActorTransform(SpawnTransform);

		this->Possess(PooledPlayer);
	}
}

void AFPSPlayerController::RespawnPlayer_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::RespawnPlayer_Implementation"));

	FTransform SpawnTransform = GameMode->GetRandomPlayerStarts(Team);
	if (GetPawn() != nullptr)
	{
		GetPawn()->SetActorTransform(SpawnTransform);
	}
}

void AFPSPlayerController::OnPlayerDeath_Implementation()
{
	GameMode->OnPlayerDeath(this, Team);
}

void AFPSPlayerController::LoadGameOver_Implementation(bool Victory)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::LoadGameOver_Implementation"));
	Client_LoadGameOver(Victory);
}

void AFPSPlayerController::Client_LoadGameOver_Implementation(bool Victory)
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	GameOverWidget = CreateWidget<UGameOverWidget>(World, GameOverWidgetClass);
	if (!ensure(GameOverWidget != nullptr))
	{
		return;
	}

	GameOverWidget->Setup();
	GameOverWidget->SetResultText(Victory);
}

void AFPSPlayerController::ShakeCamera_Implementation(TSubclassOf<UCameraShakeBase> CameraShake)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::ShakeCamera_Implementation"));
	if (CameraShake != nullptr)
	{
		ClientStartCameraShake(CameraShake);
	}
}
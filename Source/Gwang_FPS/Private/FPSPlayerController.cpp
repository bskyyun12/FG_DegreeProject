// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSPlayerController.h"
#include "Kismet/GameplayStatics.h"

#include "FPSCharacter.h"
#include "FPSGameMode.h"
#include "FPSGameStateBase.h"
#include "Widgets/TeamSelectionWidget.h"
#include "Widgets/GameOverWidget.h"
#include "Widgets/GameStatusWidget.h"

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

	GameMode->OnEndGame.RemoveAll(this);
	GameMode->OnEndGame.AddDynamic(this, &AFPSPlayerController::OnEndGame);

	GameMode->StartNewGame(this);

	Client_LoadTeamSelection();
}

#pragma region Gamemode delegate bindings
void AFPSPlayerController::OnEndGame(ETeam WinnerTeam)
{
	Client_LoadGameOver(WinnerTeam == Team);
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

#pragma endregion

#pragma region Spawn player
void AFPSPlayerController::OnTeamSelected_Implementation(ETeam InTeam)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::OnTeamSelected_Implementation()"));
	if (TeamSelection != nullptr)
	{
		TeamSelection->Teardown();
		Server_OnTeamSelected(InTeam);
	}
}

void AFPSPlayerController::Server_OnTeamSelected_Implementation(ETeam InTeam)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::Server_OnTeamSelected_Implementation()"));
	Team = InTeam;
	GameMode->SpawnPlayer(this, Team);
}

void AFPSPlayerController::OnSpawnPlayer_Implementation(AFPSCharacter* PooledPlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::OnSpawnPlayer_Implementation()"));
	if (PooledPlayer != nullptr)
	{
		FTransform SpawnTransform = GameMode->GetRandomPlayerStarts(Team);
		PooledPlayer->SetActorTransform(SpawnTransform);

		this->Possess(PooledPlayer);
	}
}
#pragma endregion

#pragma region Widgets load/unload
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

void AFPSPlayerController::UpdateTeamSelectionUI_Implementation(ETeam InTeam)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::UpdateTeamSelectionUI_Implementation()"));
	Server_UpdateTeamSelectionUI(Team);
}

void AFPSPlayerController::Server_UpdateTeamSelectionUI_Implementation(ETeam InTeam)
{
	Multicast_UpdateTeamSelectionUI(InTeam, GameMode->CanJoin(InTeam));
}

void AFPSPlayerController::Multicast_UpdateTeamSelectionUI_Implementation(ETeam InTeam, bool bCanJoin)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::Multicast_UpdateTeamSelectionUI_Implementation()"));
	if (TeamSelection != nullptr)
	{
		TeamSelection->UpdateTeamSelectionUI(InTeam, bCanJoin);
	}
}

// Game status widget(score board / tap widget)
void AFPSPlayerController::HandleGameStatusWidget_Implementation(bool bDisplay)
{
	if (bDisplay)
	{
		UWorld* World = GetWorld();
		if (!ensure(World != nullptr))
		{
			return;
		}
		GameStatusWidget = CreateWidget<UGameStatusWidget>(World, GameStatusWidgetClass);
		if (!ensure(GameStatusWidget != nullptr))
		{
			return;
		}
		GameStatusWidget->Setup(EInputMode::GameOnly, false);
	}
	else
	{
		if (GameStatusWidget != nullptr)
		{
			GameStatusWidget->Teardown();
		}
	}
}
#pragma endregion

#pragma region Death and Respawn
void AFPSPlayerController::OnPlayerDeath_Implementation()
{
	GameMode->OnPlayerDeath(this, Team);
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
#pragma endregion

void AFPSPlayerController::ShakeCamera_Implementation(TSubclassOf<UCameraShakeBase> CameraShake)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::ShakeCamera_Implementation"));
	if (CameraShake != nullptr)
	{
		ClientStartCameraShake(CameraShake);
	}
}

void AFPSPlayerController::AddControlRotation_Implementation(const FRotator& RotationToAdd)
{
	FRotator Rotation = GetControlRotation();
	SetControlRotation(Rotation + RotationToAdd);
}

ETeam AFPSPlayerController::GetTeam_Implementation()
{
	return Team;
}
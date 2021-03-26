// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

#include "FPSCharacter.h"
#include "FPSGameMode.h"
#include "FPSGameStateBase.h"
#include "Widgets/GameOverWidget.h"
#include "Widgets/GameStatusWidget.h"

void AFPSPlayerController::StartNewGame_Implementation()
{
	Client_StartNewGame();
}

void AFPSPlayerController::Client_StartNewGame_Implementation()
{
	UFPSGameInstance* GameInstance = Cast<UFPSGameInstance>(GetGameInstance());
	if (GameInstance != nullptr)
	{
		Server_StartNewGame(GameInstance->GetTeam());
	}
}

void AFPSPlayerController::Server_StartNewGame_Implementation(ETeam InTeam)
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

	/////////////
	// Doing this because I want to be able to play from FPS_Gwang map through engine!
	if (InTeam == ETeam::None)
	{
		InTeam = GameMode->GetStartingTeam();
	}
	// Doing this because I want to be able to play from FPS_Gwang map through engine!
	/////////////

	Team = InTeam;
	GameMode->SpawnPlayer(this, Team);
}

void AFPSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!ensure(GameOverWidgetClass != nullptr))
	{
		return;
	}
}

void AFPSPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSPlayerController, Team);
}

#pragma region Spawn player
void AFPSPlayerController::OnSpawnPlayer_Implementation(AFPSCharacter* PooledPlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::OnSpawnPlayer_Implementation()"));
	if (PooledPlayer != nullptr)
	{
		this->Possess(PooledPlayer);

		FTransform SpawnTransform = GameMode->GetRandomPlayerStarts(Team);
		PooledPlayer->SetActorLocation(SpawnTransform.GetLocation());

		if (HasAuthority())
		{
			ClientSetRotation(SpawnTransform.GetRotation().Rotator());
		}
	}
}
#pragma endregion

#pragma region Widgets load/unload
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

void AFPSPlayerController::LoadGameOverWidget_Implementation(ETeam WinnerTeam)
{
	Client_LoadGameOver(WinnerTeam);
}

void AFPSPlayerController::Client_LoadGameOver_Implementation(ETeam WinnerTeam)
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
	GameOverWidget->SetResultText(Team == WinnerTeam);
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
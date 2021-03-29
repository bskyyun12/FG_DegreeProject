// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

#include "FPSCharacter.h"
#include "FPSGameMode.h"
#include "FPSGameStateBase.h"
#include "Widgets/DamageReceiveWidget.h"
#include "Widgets/FPSHUDWidget.h"
#include "Widgets/GameOverWidget.h"
#include "Widgets/ScoreBoardWidget.h"

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

	Client_SetupWidgets();
}

void AFPSPlayerController::Client_SetupWidgets_Implementation()
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	if (!ensure(GameOverWidgetClass != nullptr))
	{
		return;
	}
	GameOverWidget = CreateWidget<UGameOverWidget>(World, GameOverWidgetClass);
	if (!ensure(GameOverWidget != nullptr))
	{
		return;
	}
	GameOverWidget->Setup(EInputMode::GameOnly, false);
	GameOverWidget->SetVisibility(ESlateVisibility::Hidden);

	if (!ensure(ScoreBoardWidgetClass != nullptr))
	{
		return;
	}
	ScoreBoardWidget = CreateWidget<UScoreBoardWidget>(World, ScoreBoardWidgetClass);
	if (!ensure(ScoreBoardWidget != nullptr))
	{
		return;
	}
	ScoreBoardWidget->Setup(EInputMode::GameOnly, false);
	ScoreBoardWidget->SetVisibility(ESlateVisibility::Hidden);

	if (!ensure(DamageReceiveWidgetClass != nullptr))
	{
		return;
	}
	DamageReceiveWidget = CreateWidget<UDamageReceiveWidget>(World, DamageReceiveWidgetClass);
	if (!ensure(DamageReceiveWidget != nullptr))
	{
		return;
	}
	DamageReceiveWidget->Setup(EInputMode::GameOnly, false);
	DamageReceiveWidget->SetVisibility(ESlateVisibility::Hidden);

	if (!ensure(FPSHUDWidgetClass != nullptr))
	{
		return;
	}
	FPSHUDWidget = CreateWidget<UFPSHUDWidget>(World, FPSHUDWidgetClass);
	if (!ensure(FPSHUDWidget != nullptr))
	{
		return;
	}
	FPSHUDWidget->Setup(EInputMode::GameOnly, false);
}

void AFPSPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSPlayerController, Team);
}

void AFPSPlayerController::OnSpawnPlayer_Implementation(AFPSCharacter* PooledPlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::OnSpawnPlayer_Implementation()"));
	if (HasAuthority())
	{
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
}

#pragma region Widget
void AFPSPlayerController::OnApplyDamage_Implementation()
{
	Client_OnApplyDamage();
}

void AFPSPlayerController::Client_OnApplyDamage_Implementation()
{
	if (FPSHUDWidget != nullptr)
	{
		FPSHUDWidget->OnApplyDamage();
	}
}

void AFPSPlayerController::OnTakeDamage_Implementation()
{
	// TODO: what to do here?
}

void AFPSPlayerController::OnUpdateHealthArmorUI_Implementation(bool bIsDead)
{
	Client_OnUpdateHealthArmorUI(bIsDead);
}

void AFPSPlayerController::Client_OnUpdateHealthArmorUI_Implementation(bool bIsDead)
{
	if (FPSHUDWidget != nullptr)
	{
		FPSHUDWidget->OnTakeDamage(bIsDead);
	}

	if (DamageReceiveWidget != nullptr)
	{
		UWorld* World = GetWorld();
		if (!ensure(World != nullptr))
		{
			return;
		}
		DamageReceiveWidget->SetVisibility(ESlateVisibility::Visible);
		World->GetTimerManager().ClearTimer(DamageReceiveTimer);
		World->GetTimerManager().SetTimer(DamageReceiveTimer, [&]()
			{
				DamageReceiveWidget->SetVisibility(ESlateVisibility::Hidden);
			}, 2.f, false);
	}
}

void AFPSPlayerController::OnUpdateAmmoUI_Implementation(int CurrentAmmo, int RemainingAmmo)
{
	Client_OnUpdateAmmoUI(CurrentAmmo, RemainingAmmo);
}

void AFPSPlayerController::Client_OnUpdateAmmoUI_Implementation(int CurrentAmmo, int RemainingAmmo)
{
	if (FPSHUDWidget != nullptr)
	{
		FPSHUDWidget->UpdateAmmoUI(CurrentAmmo, RemainingAmmo);
	}
}

void AFPSPlayerController::ToggleScoreBoardWidget_Implementation(bool bVisible)
{
	if (ScoreBoardWidget != nullptr)
	{
		ScoreBoardWidget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void AFPSPlayerController::LoadGameOverWidget_Implementation(ETeam WinnerTeam)
{
	Client_LoadGameOver(WinnerTeam);
}

void AFPSPlayerController::Client_LoadGameOver_Implementation(ETeam WinnerTeam)
{
	if (GameOverWidget != nullptr)
	{
		GameOverWidget->SetVisibility(ESlateVisibility::Visible);
		GameOverWidget->SetResultText(Team == WinnerTeam);
	}
}
#pragma endregion

#pragma region Death and Respawn
void AFPSPlayerController::OnPlayerDeath_Implementation()
{
	if (!ensure(HasAuthority()))
	{
		return;
	}

	FInputModeUIOnly InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	SetInputMode(InputModeData);

	GameMode->OnPlayerDeath(this, Team);
}

void AFPSPlayerController::RespawnPlayer_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::RespawnPlayer_Implementation"));

	if (HasAuthority())
	{
		FTransform SpawnTransform = GameMode->GetRandomPlayerStarts(Team);
		if (GetPawn() != nullptr)
		{
			GetPawn()->SetActorTransform(SpawnTransform);
		}
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
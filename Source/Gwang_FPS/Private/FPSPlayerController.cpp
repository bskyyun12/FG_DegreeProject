// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSPlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

#include "FPSCharacter.h"
#include "FPSGameMode.h"
#include "FPSGameState.h"
#include "Widgets/DamageReceiveWidget.h"
#include "Widgets/FPSHUDWidget.h"
#include "Widgets/GameOverWidget.h"
#include "Widgets/ScoreBoardWidget.h"

void AFPSPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSPlayerController, Team);
}

void AFPSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	Client_SetupWidgets();
}

// Bound to GameMode->OnStartMatch
void AFPSPlayerController::OnStartMatch()
{
	Server_RequestPlayerSpawn();
	Client_LoadGameOver(false, false);
}

// Bound to GameMode->OnEndMatch
void AFPSPlayerController::OnEndMatch()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::OnEndMatch_Implementation"));
	Client_LoadGameOver(Team == GameMode->GetWinnerTeam(), true);
}

#pragma region Getters
ETeam AFPSPlayerController::GetTeam_Implementation()
{
	return Team;
}
#pragma endregion Getters

#pragma region GameMode & GameState setup
// Called by AFPSPlayerController::OnPostLogin_Implementation
void AFPSPlayerController::GameModeSetup(AFPSGameMode* FPSGameMode)
{
	if (FPSGameMode != nullptr)
	{
		GameMode = FPSGameMode;

		GameMode->OnStartMatch.AddDynamic(this, &AFPSPlayerController::OnStartMatch);
		GameMode->OnEndMatch.AddDynamic(this, &AFPSPlayerController::OnEndMatch);
	}
}

// Called by AFPSPlayerController::OnPostLogin_Implementation
void AFPSPlayerController::GameStateSetup()
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	GameState = World->GetGameState<AFPSGameState>();
	if (!ensure(GameState != nullptr))
	{
		return;
	}
	GameState->OnUpdateScore.AddDynamic(this, &AFPSPlayerController::OnUpdateScoreUI);
}
#pragma endregion GameMode & GameState setup

#pragma region PostLogin Setup
void AFPSPlayerController::OnPostLogin_Implementation(AFPSGameMode* FPSGameMode)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::OnPostLogin_Implementation"));
	if (!ensure(FPSGameMode != nullptr))
	{
		return;
	}
	GameModeSetup(FPSGameMode);
	GameStateSetup();
	Client_SetupTeam();
}

void AFPSPlayerController::Client_SetupTeam_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::Client_SetupTeam_Implementation"));
	UFPSGameInstance* GameInstance = Cast<UFPSGameInstance>(GetGameInstance());
	if (GameInstance != nullptr)
	{
		Server_SetupTeam(GameInstance->GetUserData().Team);
	}
}

void AFPSPlayerController::Server_SetupTeam_Implementation(ETeam TeamFromGameInstance)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::Server_SetupTeam_Implementation"));
	Team = TeamFromGameInstance;	// Team is a replicated variable	
	Server_RequestPlayerSpawn();
}
#pragma endregion PostLogin Setup

#pragma region Spawn & Death
void AFPSPlayerController::Server_RequestPlayerSpawn_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::Server_RequestPlayerSpawn_Implementation"));
	if (!ensure(GameMode != nullptr))
	{
		return;
	}
	Team = (Team == ETeam::None) ? GameMode->GetTeamWithLessPeople() : Team;
	GameMode->SpawnPlayer(this, Team);
}

// Called by AFPSGameMode::SpawnPlayer
void AFPSPlayerController::OnSpawnPlayer_Implementation(AFPSCharacter* SpawnedPlayer)
{
	this->Possess(SpawnedPlayer);

	FTransform SpawnTransform = GameMode->GetRandomPlayerStarts(Team);
	SpawnedPlayer->SetActorLocation(SpawnTransform.GetLocation());

	if (HasAuthority())
	{
		this->ClientSetRotation(SpawnTransform.GetRotation().Rotator());
	}

	if (GetPawn() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetPawn(), UFPSCharacterInterface::StaticClass()))
	{
		IFPSCharacterInterface::Execute_OnSpawnPlayer(GetPawn());
	}
}

// Called by FPSCharacter::OnDeath
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
#pragma endregion Spawn & Death

#pragma region Widget
// Called by AFPSPlayerController::BeginPlay()
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
	Client_OnTakeDamage();
}

void AFPSPlayerController::Client_OnTakeDamage_Implementation()
{
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

void AFPSPlayerController::OnUpdateScoreUI(int MarvelScore, int DCScore)
{
	Client_OnUpdateScoreUI(MarvelScore, DCScore);
}

void AFPSPlayerController::Client_OnUpdateScoreUI_Implementation(int MarvelScore, int DCScore)
{
	if (FPSHUDWidget != nullptr)
	{
		FPSHUDWidget->UpdateScoreUI(MarvelScore, DCScore);
	}
}

void AFPSPlayerController::ToggleScoreBoardWidget_Implementation(bool bVisible)
{
	if (ScoreBoardWidget != nullptr)
	{
		ScoreBoardWidget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void AFPSPlayerController::Client_LoadGameOver_Implementation(bool bIsWinner, bool bWidgetVisibility)
{
	if (GameOverWidget != nullptr)
	{
		GameOverWidget->SetVisibility(bWidgetVisibility ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
		GameOverWidget->SetResultText(bIsWinner);
	}
}
#pragma endregion Widget

#pragma region Recoil
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
#pragma endregion Recoil
// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchPlayerController.h"
#include "Net/UnrealNetwork.h"

#include "FPSGameInstance.h"
#include "DeathMatchGameMode.h"
#include "DeathMatchCharacter.h"
#include "Widgets/FPSHUDWidget.h"
#include "DeathMatchPlayerState.h"
#include "Widgets/ScoreBoardWidget.h"
#include "Widgets/GameOverWidget.h"
#include "Widgets/VignetteWidget.h"

void ADeathMatchPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ADeathMatchPlayerController::Server_OnPostLogin_Implementation()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		UWorld* World = GetWorld();
		if (!ensure(World != nullptr))
		{
			return;
		}
		GM = Cast<ADeathMatchGameMode>(World->GetAuthGameMode());
		if (!ensure(GM != nullptr))
		{
			return;
		}

		World->GetTimerManager().SetTimer(ReadyCheckTimer, this, &ADeathMatchPlayerController::Client_CheckReadyStatus, .5f, true);
	}

	Client_SetupWidgets();
}

void ADeathMatchPlayerController::Client_CheckReadyStatus_Implementation()
{
	// TODO: make a getter for PS? like how I did in Character script
	PS = GetPlayerState<ADeathMatchPlayerState>();
	if (PS != nullptr)
	{
		if (PS->GetIsReadyToJoin() && bWidgetLoaded)
		{
			Server_RequestPlayerSpawn();
		}
	}
}

void ADeathMatchPlayerController::Server_RequestPlayerSpawn_Implementation()
{
	GM->SpawnPlayer(this);

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	World->GetTimerManager().ClearTimer(ReadyCheckTimer);
}

// Called after ADeathMatchGameMode::SpawnPlayer
void ADeathMatchPlayerController::Server_OnSpawnPlayer_Implementation(ADeathMatchCharacter* SpawnedPlayer)
{
	SpawnedPlayer->Server_OnSpawnPlayer();
	Client_OnSpawnPlayer();
}

void ADeathMatchPlayerController::Client_OnSpawnPlayer_Implementation()
{
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
}

// Called after ADeathMatchCharacter::Server_OnDeath
void ADeathMatchPlayerController::OnPlayerDeath_Implementation()
{
	Server_OnPlayerDeath();
}

void ADeathMatchPlayerController::Server_OnPlayerDeath_Implementation()
{
	GM->OnPlayerDeath(this);
	Client_OnPlayerDeath();
}

void ADeathMatchPlayerController::Client_OnPlayerDeath_Implementation()
{
	FInputModeUIOnly InputMode;
	SetInputMode(InputMode);
}

#pragma region Widget Related
void ADeathMatchPlayerController::Client_SetupWidgets_Implementation()
{
	bWidgetLoaded = true;

	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		// HUD widget
		if (HUDWidget == nullptr)
		{
			if (!ensure(HUDWidgetClass != nullptr))
			{
				return;
			}
			HUDWidget = CreateWidget<UFPSHUDWidget>(World, HUDWidgetClass);
			if (!ensure(HUDWidget != nullptr))
			{
				return;
			}
			HUDWidget->AddToViewport();
		}

		// Scoreboard widget
		if (ScoreboardWidget == nullptr)
		{
			if (!ensure(ScoreboardWidgetClass != nullptr))
			{
				return;
			}
			ScoreboardWidget = CreateWidget<UScoreBoardWidget>(World, ScoreboardWidgetClass);
			if (!ensure(ScoreboardWidget != nullptr))
			{
				return;
			}
			ScoreboardWidget->AddToViewport(1);
			ScoreboardWidget->SetVisibility(ESlateVisibility::Hidden);
		}

		// Vignette Widget
		if (VignetteWidget == nullptr)
		{
			if (!ensure(VignetteWidgetClass != nullptr))
			{
				return;
			}
			VignetteWidget = CreateWidget<UVignetteWidget>(World, VignetteWidgetClass);
			if (!ensure(VignetteWidget != nullptr))
			{
				return;
			}
			VignetteWidget->AddToViewport(2);
		}

		// Gameover Widget
		if (GameOverWidget == nullptr)
		{
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
		}
	}
}

void ADeathMatchPlayerController::UpdateMatchTimeUI(const float& MatchTime)
{

}

void ADeathMatchPlayerController::UpdateScoreUI(const uint8& MarvelScore, const uint8& DCScore)
{
	HUDWidget->UpdateScoreUI(MarvelScore, DCScore);
}

void ADeathMatchPlayerController::ChangeCrosshairUIOnHit_Implementation()
{
	HUDWidget->UpdateCrosshairUIOnHit();
}

void ADeathMatchPlayerController::VignetteEffectOnTakeDamage_Implementation()
{
	VignetteWidget->OnTakeDamage();
}

void ADeathMatchPlayerController::SendChat(const FName& ChatContent)
{
	PS->Server_OnSendChat(ChatContent);
}

void ADeathMatchPlayerController::UpdateChatUI(const FName& PlayerName, const FName& ChatContent)
{
	HUDWidget->AddChatRow(PlayerName, ChatContent);
}

void ADeathMatchPlayerController::UpdateWeaponUI_Implementation(const FName& WeaponName, const int& CurrentAmmo, const int& RemainingAmmo)
{
	HUDWidget->UpdateWeaponUI(WeaponName, CurrentAmmo, RemainingAmmo);
}

void ADeathMatchPlayerController::LoadGameOverUI(const bool& bIsWinner, const bool& bWidgetVisibility)
{
	GameOverWidget->SetVisibility(bWidgetVisibility ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	GameOverWidget->SetResultText(bIsWinner);
}

void ADeathMatchPlayerController::SetScoreBoardUIVisibility_Implementation(bool bNewVisibility)
{
	ScoreboardWidget->SetVisibility(bNewVisibility ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void ADeathMatchPlayerController::UpdateHealthArmorUI(const uint8& CurrentHealth, const uint8& CurrentArmor)
{
	HUDWidget->UpdateHealthArmorUI(CurrentHealth, CurrentArmor);
}

void ADeathMatchPlayerController::StartChat_Implementation()
{
	HUDWidget->OnStartChat();
}

#pragma endregion


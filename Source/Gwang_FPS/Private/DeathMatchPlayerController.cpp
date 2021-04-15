// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchPlayerController.h"
#include "Net/UnrealNetwork.h"

#include "FPSGameInstance.h"
#include "DeathMatchGameMode.h"
#include "Widgets/FPSHUDWidget.h"
#include "DeathMatchPlayerState.h"
#include "Widgets/DamageReceiveWidget.h"
#include "Widgets/ScoreBoardWidget.h"
#include "Widgets/GameOverWidget.h"
#include "DeathMatchCharacter.h"

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
	if (PS == nullptr)
	{
		return;
	}

	if (PS->GetIsReadyToJoin() && bWidgetLoaded)
	{
		Server_RequestPlayerSpawn();
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
void ADeathMatchPlayerController::Server_OnSpawnPlayer_Implementation(ADeathMatchCharacter* DM_Player)
{
	DM_Player->Server_OnSpawnPlayer();
}

void ADeathMatchPlayerController::BeginPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("(GameFlow) PlayerController::BeginPlay (%s)"), *GetName());
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
}

#pragma region Widget Related
void ADeathMatchPlayerController::Client_SetupWidgets_Implementation()
{
	bWidgetLoaded = true;

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

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
		if (VignetteWidget != nullptr)
		{
			if (!ensure(VignetteWidgetClass != nullptr))
			{
				return;
			}
			VignetteWidget = CreateWidget<UDamageReceiveWidget>(World, VignetteWidgetClass);
			if (!ensure(VignetteWidget != nullptr))
			{
				return;
			}
			VignetteWidget->AddToViewport(2);
			// TODO: Change to widget animation fade in and out
			VignetteWidget->SetVisibility(ESlateVisibility::Hidden);
		}

		// Gameover Widget
		if (GameOverWidget != nullptr)
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

void ADeathMatchPlayerController::UpdateCrosshairUIOnHit()
{
	HUDWidget->UpdateCrosshairUIOnHit();
}

void ADeathMatchPlayerController::VignetteUIOnTakeDamage()
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	// TODO: Change to widget animation fade in and out
	VignetteWidget->SetVisibility(ESlateVisibility::Visible);
	World->GetTimerManager().SetTimer(VignetteTimer, [&]()
		{
			VignetteWidget->SetVisibility(ESlateVisibility::Hidden);
		}, 2.f, false);
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


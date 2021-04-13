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

void ADeathMatchPlayerController::OnPostLogin()
{
	PS = GetPlayerState<ADeathMatchPlayerState>();
	if (!ensure(PS != nullptr))
	{
		return;
	}
	PS->OnPostLogin();
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

	if (GetLocalRole() == ROLE_Authority)
	{
		GM = Cast<ADeathMatchGameMode>(World->GetAuthGameMode());
		if (!ensure(GM != nullptr))
		{
			return;
		}
	}

	Client_SetupWidgets();
}

void ADeathMatchPlayerController::OnPossess(APawn* aPawn)
{
	UE_LOG(LogTemp, Warning, TEXT("(GameFlow) PlayerController::OnPossess (%s)"), *GetName());
	Super::OnPossess(aPawn);

	Client_OnPossess();
}

void ADeathMatchPlayerController::Client_OnPossess_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchPlayerController::Client_OnPossess => Role: ( %i )"), GetLocalRole());

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
}

#pragma region Widget

void ADeathMatchPlayerController::Client_SetupWidgets_Implementation()
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	// HUD widget
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

	// Scoreboard widget
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

	// Vignette Widget
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

	// Gameover Widget
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


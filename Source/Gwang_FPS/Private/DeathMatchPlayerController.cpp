// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchPlayerController.h"
#include "Net/UnrealNetwork.h"

#include "FPSGameInstance.h"
#include "DeathMatchGameMode.h"
#include "Widgets/FPSHUDWidget.h"
#include "DeathMatchPlayerState.h"
#include "Widgets/DamageReceiveWidget.h"
#include "Widgets/ScoreBoardWidget.h"

void ADeathMatchPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
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

	FTimerHandle DelayTimer;
	World->GetTimerManager().SetTimer(DelayTimer, [&]()
		{
			PS = Cast<ADeathMatchPlayerState>(PlayerState);
			if (!ensure(PS != nullptr))
			{
				return;
			}
		}, 0.5f, false);
}

void ADeathMatchPlayerController::OnPossess(APawn* aPawn)
{
	UE_LOG(LogTemp, Warning, TEXT("(GameFlow) PlayerController::OnPossess (%s)"), *GetName());
	Super::OnPossess(aPawn);

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	Client_OnPossess();
}

void ADeathMatchPlayerController::Client_OnPossess_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("(GameFlow) PlayerController::Client_OnPossess (%s)"), *GetName());
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
}

#pragma region Widget
void ADeathMatchPlayerController::UpdateMatchTimeUI(const float& MatchTime)
{

}

void ADeathMatchPlayerController::UpdateAmmoUI(const uint16& CurrentAmmo, const uint16& RemainingAmmo)
{
	HUDWidget->UpdateAmmoUI(CurrentAmmo, RemainingAmmo);
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

void ADeathMatchPlayerController::SetScoreBoardUIVisibility(const bool& NewVisibility)
{
	ScoreboardWidget->SetVisibility(NewVisibility ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void ADeathMatchPlayerController::StartChat()
{
	HUDWidget->OnStartChat();
}

void ADeathMatchPlayerController::SendChat(const FName& PlayerName, const FName& ChatContent)
{
	PS->Server_OnSendChat(PlayerName, ChatContent);
}

void ADeathMatchPlayerController::UpdateChatUI(const FName& PlayerName, const FName& ChatContent)
{
	HUDWidget->AddChatRow(PlayerName, ChatContent);
}

#pragma endregion


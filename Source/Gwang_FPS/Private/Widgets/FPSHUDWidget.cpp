// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/FPSHUDWidget.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Kismet/KismetTextLibrary.h"

#include "Widgets/HealthArmorWidget.h"
#include "Widgets/ChatPanel.h"
#include "Widgets/AmmoWidget.h"

void UFPSHUDWidget::UpdateCrosshairUIOnHit()
{
	// Crosshair UI
	WidgetSwitcher_Crosshair->SetActiveWidgetIndex(1);

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	World->GetTimerManager().SetTimer(CrosshairTimer, [&]()
		{
			WidgetSwitcher_Crosshair->SetActiveWidgetIndex(0);
		}, 0.2f, false);
}

void UFPSHUDWidget::UpdateHealthArmorUI(const uint8& Health, const uint8& Armor)
{
	HealthArmorWidget->UpdateHealthArmorUI(Health, Armor);
}

void UFPSHUDWidget::UpdateWeaponUI(const FName& WeaponName, const uint16& CurrentAmmo, const uint16& RemainingAmmo)
{
	AmmoWidget->UpdateWeaponUI(WeaponName, CurrentAmmo, RemainingAmmo);
}

void UFPSHUDWidget::UpdateScoreUI(int MarvelScore, int DCScore)
{
	Text_MarvelScore->SetText(UKismetTextLibrary::Conv_IntToText(MarvelScore));
	Text_DCScore->SetText(UKismetTextLibrary::Conv_IntToText(DCScore));
}

void UFPSHUDWidget::OnStartChat()
{
	ChatPanel->OnStartChat();
}

void UFPSHUDWidget::UpdateMatchTime(const uint8& Minutes, const uint8& Seconds)
{
	Text_Minutes->SetText(UKismetTextLibrary::Conv_IntToText(Minutes));
	Text_Seconds->SetText(UKismetTextLibrary::Conv_IntToText(Seconds));
}

void UFPSHUDWidget::AddChatRow(const FName& PlayerName, const FName& ChatContent)
{
	ChatPanel->AddChatRow(PlayerName, ChatContent);
}


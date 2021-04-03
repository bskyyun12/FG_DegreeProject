// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/FPSHUDWidget.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Kismet/KismetTextLibrary.h"

#include "Widgets/HealthArmorWidget.h"

void UFPSHUDWidget::OnApplyDamage()
{
	// Crosshair UI
	WidgetSwitcher_Crosshair->SetActiveWidgetIndex(1);

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	World->GetTimerManager().ClearTimer(CrosshairTimer);
	World->GetTimerManager().SetTimer(CrosshairTimer, [&]()
		{
			WidgetSwitcher_Crosshair->SetActiveWidgetIndex(0);
		}, 0.2f, false);
}

void UFPSHUDWidget::OnTakeDamage(bool bIsDead)
{
	HealthArmorWidget->UpdateUI(bIsDead);
}

void UFPSHUDWidget::UpdateAmmoUI(int CurrentAmmo, int RemainingAmmo)
{
	AmmoWidget->UpdateUI(CurrentAmmo, RemainingAmmo);
}

void UFPSHUDWidget::UpdateScoreUI(int MarvelScore, int DCScore)
{
	Text_MarvelScore->SetText(UKismetTextLibrary::Conv_IntToText(MarvelScore));
	Text_DCScore->SetText(UKismetTextLibrary::Conv_IntToText(DCScore));
}

UChatPanel* UFPSHUDWidget::GetChatPanel() const
{
	return ChatPanel;
}

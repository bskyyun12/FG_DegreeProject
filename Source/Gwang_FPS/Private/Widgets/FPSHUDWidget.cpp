// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/FPSHUDWidget.h"
#include "Components/TextBlock.h"

#include "Widgets/HealthArmorWidget.h"

void UFPSHUDWidget::OnApplyDamage()
{
	Text_Crosshair->SetColorAndOpacity(Crosshair_OnHit);

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	World->GetTimerManager().ClearTimer(CrosshairTimer);
	World->GetTimerManager().SetTimer(CrosshairTimer, [&]()
		{
			Text_Crosshair->SetColorAndOpacity(Crosshair_Default);
		}, 0.15f, false);
}

void UFPSHUDWidget::OnTakeDamage(bool bIsDead)
{
	HealthArmorWidget->UpdateUI(bIsDead);
}

void UFPSHUDWidget::UpdateAmmoUI(int CurrentAmmo, int RemainingAmmo)
{
	AmmoWidget->UpdateUI(CurrentAmmo, RemainingAmmo);
}

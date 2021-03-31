// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/FPSWidgetBase.h"
#include "FPSHUDWidget.generated.h"

class UTextBlock;
class UAmmoWidget;
class UHealthArmorWidget;
class UWidgetSwitcher;

UCLASS()
class GWANG_FPS_API UFPSHUDWidget : public UFPSWidgetBase
{
	GENERATED_BODY()

public:
	void OnApplyDamage();

	void OnTakeDamage(bool bIsDead);

	void UpdateAmmoUI(int CurrentAmmo, int RemainingAmmo);

	void UpdateScoreUI(int MarvelScore, int DCScore);

private:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_MarvelScore;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_DCScore;

	UPROPERTY(meta = (BindWidget))
	UAmmoWidget* AmmoWidget;

	UPROPERTY(meta = (BindWidget))
	UHealthArmorWidget* HealthArmorWidget;

	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* WidgetSwitcher_Crosshair;

	FTimerHandle CrosshairTimer;
};

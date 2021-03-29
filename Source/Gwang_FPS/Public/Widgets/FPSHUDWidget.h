// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/FPSWidgetBase.h"
#include "FPSHUDWidget.generated.h"

class UTextBlock;
class UAmmoWidget;
class UHealthArmorWidget;

UCLASS()
class GWANG_FPS_API UFPSHUDWidget : public UFPSWidgetBase
{
	GENERATED_BODY()

public:
	void OnApplyDamage();

	void OnTakeDamage(bool bIsDead);

	void UpdateAmmoUI(int CurrentAmmo, int RemainingAmmo);

private:
	UPROPERTY(meta=(BindWidget))
	UTextBlock* Text_Crosshair;

	UPROPERTY(meta = (BindWidget))
	UAmmoWidget* AmmoWidget;

	UPROPERTY(meta = (BindWidget))
	UHealthArmorWidget* HealthArmorWidget;
	
	UPROPERTY(EditDefaultsOnly)
	FSlateColor Crosshair_Default;

	UPROPERTY(EditDefaultsOnly)
	FSlateColor Crosshair_OnHit;

	FTimerHandle CrosshairTimer;
};

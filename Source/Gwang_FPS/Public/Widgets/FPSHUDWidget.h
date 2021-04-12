// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPSHUDWidget.generated.h"

class UTextBlock;
class UAmmoWidget;
class UHealthArmorWidget;
class UWidgetSwitcher;
class UChatPanel;

UCLASS()
class GWANG_FPS_API UFPSHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateCrosshairUIOnHit();

	void UpdateHealthArmorUI(const uint8& Health, const uint8& Armor);

	void UpdateWeaponUI(const FName& WeaponName, const uint16& CurrentAmmo, const uint16& RemainingAmmo);

	void UpdateScoreUI(int MarvelScore, int DCScore);

	UChatPanel* GetChatPanel() const { return ChatPanel; }

	void AddChatRow(const FName& PlayerName, const FName& ChatContent);
	void OnStartChat();
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

	UPROPERTY(meta = (BindWidget))
	UChatPanel* ChatPanel;

	FTimerHandle CrosshairTimer;
};

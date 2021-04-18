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
	// Team Score
	void UpdateScoreUI(int MarvelScore, int DCScore);

	// Weapon ( Name, Ammo )
	void UpdateWeaponUI(const FName& WeaponName, const uint16& CurrentAmmo, const uint16& RemainingAmmo);

	// Health & Armor
	void UpdateHealthArmorUI(const uint8& Health, const uint8& Armor);

	// Crosshair
	void UpdateCrosshairUIOnHit();

	// Chat
	UChatPanel* GetChatPanel() const { return ChatPanel; }
	void AddChatRow(const FName& PlayerName, const FName& ChatContent);
	void OnStartChat();

	// Match Time Left
	void UpdateMatchTime(const uint8& Minutes, const uint8& Seconds);

private:
	// Team Score
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_MarvelScore;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_DCScore;

	// Weapon ( Name, Ammo )
	UPROPERTY(meta = (BindWidget))
	UAmmoWidget* AmmoWidget;

	// Health & Armor
	UPROPERTY(meta = (BindWidget))
	UHealthArmorWidget* HealthArmorWidget;

	// Crosshair
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* WidgetSwitcher_Crosshair;

	FTimerHandle CrosshairTimer;

	// Chat
	UPROPERTY(meta = (BindWidget))
	UChatPanel* ChatPanel;

	// Match Time Left
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Minutes;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Seconds;
};

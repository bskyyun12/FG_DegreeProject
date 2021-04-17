// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/FPSWidgetBase.h"
#include "LobbyInventory.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class GWANG_FPS_API ULobbyInventory : public UFPSWidgetBase
{
	GENERATED_BODY()

public:
	bool Initialize() override;

private:
	// MainWeapon
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_MainWeapon;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_MainWeaponLeft;
	UFUNCTION()
	void OnClicked_Button_MainWeaponLeft();

	UPROPERTY(meta = (BindWidget))
	UButton* Button_MainWeaponRight;
	UFUNCTION()
	void OnClicked_Button_MainWeaponRight();

	// SubWeapon
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_SubWeapon;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_SubWeaponLeft;
	UFUNCTION()
	void OnClicked_Button_SubWeaponLeft();

	UPROPERTY(meta = (BindWidget))
	UButton* Button_SubWeaponRight;
	UFUNCTION()
	void OnClicked_Button_SubWeaponRight();

	// MeleeWeapon
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_MeleeWeapon;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_MeleeWeaponLeft;
	UFUNCTION()
	void OnClicked_Button_MeleeWeaponLeft();

	UPROPERTY(meta = (BindWidget))
	UButton* Button_MeleeWeaponRight;
	UFUNCTION()
	void OnClicked_Button_MeleeWeaponRight();

	// Grenade
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Grenade;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_GrenadeLeft;
	UFUNCTION()
	void OnClicked_Button_GrenadeLeft();

	UPROPERTY(meta = (BindWidget))
	UButton* Button_GrenadeRight;
	UFUNCTION()
	void OnClicked_Button_GrenadeRight();
			
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Apply;
	UFUNCTION()
	void OnClicked_Button_Apply();

	void ChangeWeaponText(int& Index, UEnum* WeaponEnum, UTextBlock* WeaponText);

	UEnum* MainWeaponEnum;
	UEnum* SubWeaponEnum;
	UEnum* MeleeWeaponEnum;
	UEnum* GrenadeEnum;

	int MainWeaponIndex;
	int SubWeaponIndex;
	int MeleeWeaponIndex;
	int GrenadeIndex;
};

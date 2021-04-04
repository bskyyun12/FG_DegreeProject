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
		
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Apply;
	UFUNCTION()
	void OnClicked_Button_Apply();

	UEnum* MainWeaponEnum;
	UEnum* SubWeaponEnum;

	int CurrentStartMainWeaponIndex;
	int CurrentStartSubWeaponIndex;
};

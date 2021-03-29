// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/FPSWidgetBase.h"
#include "AmmoWidget.generated.h"

class UTextBlock;

UCLASS()
class GWANG_FPS_API UAmmoWidget : public UFPSWidgetBase
{
	GENERATED_BODY()
public:
	void UpdateUI(int Ammo);

private:	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_CurrentAmmo;
};

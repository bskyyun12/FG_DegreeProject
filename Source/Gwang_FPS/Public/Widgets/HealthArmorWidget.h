// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/FPSWidgetBase.h"
#include "HealthArmorWidget.generated.h"

class UTextBlock;

UCLASS()
class GWANG_FPS_API UHealthArmorWidget : public UFPSWidgetBase
{
	GENERATED_BODY()
	
public:
	void UpdateUI(bool bIsDead);

private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Health;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Armor;
};

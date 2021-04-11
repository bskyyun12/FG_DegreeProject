// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthArmorWidget.generated.h"

class UTextBlock;

UCLASS()
class GWANG_FPS_API UHealthArmorWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void UpdateHealthArmorUI(const uint8& Health, const uint8& Armor);

private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Health;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Armor;
};

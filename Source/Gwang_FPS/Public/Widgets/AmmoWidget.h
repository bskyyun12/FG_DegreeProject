// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AmmoWidget.generated.h"

class UTextBlock;

UCLASS()
class GWANG_FPS_API UAmmoWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void UpdateAmmoUI(const uint16& CurrentAmmo, const uint16& RemainingAmmo);

private:	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_CurrentAmmo;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_RemainingAmmo;
};

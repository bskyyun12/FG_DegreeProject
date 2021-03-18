// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/FPSWidgetBase.h"
#include "MainMenuWidget.generated.h"

class UFPSGameInstance;
class UButton;

UCLASS()
class GWANG_FPS_API UMainMenuWidget : public UFPSWidgetBase
{
	GENERATED_BODY()
	
public:
	void Setup(EInputMode InputMode = EInputMode::UIOnly, bool bShowCursor = true) override;

private:
	UFPSGameInstance* FPSGameInstance;
	
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Host;

	UFUNCTION()
	void OnClicked_Button_Host();
};

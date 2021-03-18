// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/FPSWidgetBase.h"
#include "MainMenuWidget.generated.h"

class UFPSGameInstance;
class UButton;
class UUserWidget;
class UScrollBox;

UCLASS()
class GWANG_FPS_API UMainMenuWidget : public UFPSWidgetBase
{
	GENERATED_BODY()
	
public:
	void Setup(EInputMode InputMode = EInputMode::UIOnly, bool bShowCursor = true) override;
	void SetSessionInfoRowClass(TSubclassOf<UUserWidget> InSessionInfoRowClass);

private:
	UFPSGameInstance* FPSGameInstance;

	TSubclassOf<UUserWidget> SessionInfoRowClass;
	
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ScrollBox_SessionList;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Host;
	UFUNCTION()
	void OnClicked_Button_Host();

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Find;
	UFUNCTION()
	void OnClicked_Button_Find();

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Join;
	UFUNCTION()
	void OnClicked_Button_Join();
};

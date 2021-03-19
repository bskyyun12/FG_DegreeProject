// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/FPSWidgetBase.h"
#include "SessionInfoRow.generated.h"

class UMainMenuWidget;
class UButton;
class UTextBlock;

UCLASS()
class GWANG_FPS_API USessionInfoRow : public UFPSWidgetBase
{
	GENERATED_BODY()

public:
	void InitializeRow(UMainMenuWidget* MainMenuWidget, int Index);
	void SetSessionName(const FString& SessionName);

private:
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Row;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_SessionName;

	UFUNCTION()
	void OnClicked_Button_Row();

	UPROPERTY()
	UMainMenuWidget* MainMenu;

	UPROPERTY()
	int RowIndex;

	UFUNCTION()
	void OnUpdateUI(int SelectedIndex);

	UPROPERTY(EditAnywhere)
	FLinearColor TextColor_Default;

	UPROPERTY(EditAnywhere)
	FLinearColor TextColor_Selected;
};

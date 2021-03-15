// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/FPSWidgetBase.h"
#include "GameOverWidget.generated.h"

class UWidgetSwitcher;
class UButton;

UCLASS()
class GWANG_FPS_API UGameOverWidget : public UFPSWidgetBase
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

	void SetResultText(bool Victory);

private:
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* WidgetSwitcher_Result;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Restart;

private:
	UFUNCTION()
	void OnClick_Button_Restart();
};

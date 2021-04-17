// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/FPSWidgetBase.h"
#include "MenuError.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class GWANG_FPS_API UMenuError : public UFPSWidgetBase
{
	GENERATED_BODY()
	
public:
	bool Initialize() override;

	void UpdateUI(const FString& Title, const FString& ErrorMsg, bool bEnableCloseButton = true, const float& Duration = -1.f);

private:
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Error;
	UFUNCTION()
	void OnClick_Button_Error();

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Title;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_ErrorMsg;
};

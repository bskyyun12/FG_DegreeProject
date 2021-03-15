// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPSWidgetBase.h"
#include "TeamSelectionWidget.generated.h"

class UButton;
class APlayerController;

UCLASS()
class GWANG_FPS_API UTeamSelectionWidget : public UFPSWidgetBase
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

private:
	UPROPERTY(meta = (BindWidget))
	UButton* Button_MarvelTeam;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_DCTeam;

private:
	UFUNCTION()
	void OnClick_Button_MarvelTeam();

	UFUNCTION()
	void OnClick_Button_DCTeam();
};

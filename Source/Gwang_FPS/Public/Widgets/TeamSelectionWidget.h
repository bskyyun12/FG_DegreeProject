// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPSWidgetBase.h"
#include "FPSPlayerStart.h"
#include "TeamSelectionWidget.generated.h"

class UButton;
class UImage;
class AFPSGameStateBase;

UCLASS()
class GWANG_FPS_API UTeamSelectionWidget : public UFPSWidgetBase
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

	void Setup(EInputMode InputMode = EInputMode::UIOnly, bool bShowCursor = true) override;

	void Teardown() override;

	UFUNCTION()
	void UpdateTeamSelectionUI(ETeam Team, bool bCanJoin);

private:
	UPROPERTY(meta = (BindWidget))
	UButton* Button_MarvelTeam;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_DCTeam;

	UPROPERTY(meta = (BindWidget))
	UImage* Image_MarvelTeam;

	UPROPERTY(meta = (BindWidget))
	UImage* Image_DCTeam;

	FTimerHandle UpdateTimer;

private:
	UFUNCTION()
	void OnClick_Button_MarvelTeam();

	UFUNCTION()
	void OnClick_Button_DCTeam();

	UFUNCTION()
	void RequestUIUpdate();

	AFPSGameStateBase* FPSGameState;
};

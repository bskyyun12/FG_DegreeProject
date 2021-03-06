// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuInterface.h"
#include "TeamSelectionWidget.generated.h"

/**
 * 
 */
class UButton;

UCLASS()
class GWANG_FPS_API UTeamSelectionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

	void Setup();

	void Teardown();

private:

	UPROPERTY(meta = (BindWidget))
	UButton* Button_DarkTeam;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_SilverTeam;

	UFUNCTION()
	void OnClick_Button_DarkTeam();

	UFUNCTION()
	void OnClick_Button_SilverTeam();
};

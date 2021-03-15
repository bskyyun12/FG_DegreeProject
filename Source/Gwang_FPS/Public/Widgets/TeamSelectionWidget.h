// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPSWidgetBase.h"
#include "FPSPlayerStart.h"
#include "TeamSelectionWidget.generated.h"

class UButton;
class UImage;
class APlayerController;

UCLASS()
class GWANG_FPS_API UTeamSelectionWidget : public UFPSWidgetBase
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

	void Setup() override;

	void Teardown() override;

private:
	UPROPERTY(meta = (BindWidget))
	UButton* Button_MarvelTeam;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_DCTeam;

	UPROPERTY(meta = (BindWidget))
	UImage* Image_MarvelTeam;

	UPROPERTY(meta = (BindWidget))
	UImage* Image_DCTeam;

	UPROPERTY()
	AFPSGameMode* GameMode;

	UPROPERTY(Replicated)
	bool bCanJoinMarvelTeam = true;

	UPROPERTY(Replicated)
	bool bCanJoinDCTeam = true;

	FTimerHandle RefreshTimer;

private:

	UFUNCTION()
	void OnClick_Button_MarvelTeam();

	UFUNCTION()
	void OnClick_Button_DCTeam();

	UFUNCTION(Client, Reliable)
	void Client_Refresh();

	UFUNCTION(Server, Reliable)
	void Server_Refresh();
};

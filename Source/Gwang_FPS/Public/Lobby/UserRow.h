// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/FPSWidgetBase.h"
#include "LobbyGameMode.h"
#include "UserRow.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class GWANG_FPS_API UUserRow : public UFPSWidgetBase
{
	GENERATED_BODY()
	
public:
	bool Initialize() override;

	void UpdateRow(const FLobbyPlayerData& Data);

private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_UserName;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_ID;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Ready;
	UFUNCTION()
	void OnClicked_Button_Ready();

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Ready;

	UPROPERTY(EditAnywhere)
	FSlateColor TextColor_Default;

	UPROPERTY(EditAnywhere)
	FSlateColor TextColor_Ready;

	bool bIsReady;
};

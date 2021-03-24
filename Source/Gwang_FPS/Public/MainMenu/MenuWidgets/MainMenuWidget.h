// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/FPSWidgetBase.h"
#include "MainMenuWidget.generated.h"

class UFPSGameInstance;
class UWidgetSwitcher;
class UButton;
class UUserWidget;
class UScrollBox;
class UEditableTextBox;

USTRUCT()
struct FServerData
{
	GENERATED_BODY()

	uint16 Index;
	FString ServerName;
	uint16 CurrentPlayers;
	uint16 MaxPlayers;
	FString HostUsername;

	FServerData()
	{
		Index = 0;
		ServerName = "Server Name";
		CurrentPlayers = 0;
		MaxPlayers = 0;
		HostUsername = "Gwang";
	}
};

UCLASS()
class GWANG_FPS_API UMainMenuWidget : public UFPSWidgetBase
{
	GENERATED_BODY()
	
public:
	void Setup(EInputMode InputMode = EInputMode::UIOnly, bool bShowCursor = true) override;

	void UpdateSessionList(TArray<FServerData> ServerData);

	void SetSelectIndex(int Index);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSessionListDelegate, int, SelectedIndex);
	FSessionListDelegate OnUpdateUI;

private:
	UFPSGameInstance* FPSGameInstance;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> SessionInfoRowClass;

	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* WidgetSwitcher_MainMenu;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* ScrollBox_SessionList;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* EditableText_ServerName;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Create;
	UFUNCTION()
	void OnClicked_Button_Create();

	UPROPERTY(meta = (BindWidget))
	UButton* Button_BackToMenu;
	UFUNCTION()
	void OnClicked_Button_BackToMenu();

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

	int SelectedIndex;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/FPSWidgetBase.h"
#include "MainMenuWidget.generated.h"

class UFPSGameInstance;
class UWidgetSwitcher;
class UMenuError;
class UButton;
class UUserWidget;
class UScrollBox;
class UEditableTextBox;

USTRUCT()
struct FSessionInfoData
{
	GENERATED_BODY()

	uint16 Index;
	FName ServerName;
	FName HostUsername;
	uint16 CurrentPlayers;
	uint16 MaxPlayers;
	uint16 Ping;

	FSessionInfoData()
	{
		Index = 0;
		ServerName = "Server Name";
		HostUsername = "Gwang";
		CurrentPlayers = 0;
		MaxPlayers = 0;
		Ping = 0;
	}
};

USTRUCT()
struct FSessionCreationData
{
	GENERATED_BODY()

	bool bIsLan;
	int NumPublicConnections;
	FName SessionName;

};

UCLASS()
class GWANG_FPS_API UMainMenuWidget : public UFPSWidgetBase
{
	GENERATED_BODY()
	
public:
	void Setup(EInputMode InputMode = EInputMode::UIOnly, bool bShowCursor = true) override;

	void UpdateSessionList(TArray<FSessionInfoData> ServerData);

	void SetSelectIndex(int Index);

	void ShowErrorWidget(const FString& Title, const FString& ErrorMsg, bool bEnableCloseButton = true, const float& Duration = -1.f);

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
	UMenuError* ErrorWidget;

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

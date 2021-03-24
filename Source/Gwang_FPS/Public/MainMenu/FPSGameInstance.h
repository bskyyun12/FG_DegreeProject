// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "OnlineSubSystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MenuWidgets/MainMenuInterface.h"

#include "FPSGameInstance.generated.h"

class UUserWidget;
class UMainMenuWidget;
class FOnlineSessionSearch;

UCLASS()
class GWANG_FPS_API UFPSGameInstance : public UGameInstance, public IMainMenuInterface
{
	GENERATED_BODY()
	
public:
	UFPSGameInstance(const FObjectInitializer& ObjectIn);

	void Init() override;

	void SetMainMenu(UMainMenuWidget* InMainMenu);

	void LoadMainMenu(TSubclassOf<UUserWidget> MainMenuWidgetClass, TSubclassOf<UUserWidget> SessionInfoRowClass);

	void StartSession();

	//////////////////////
	// IMainMenuInterface
	void Host_Implementation(const FString& InServerName) override;
	void Find_Implementation() override;
	void Join_Implementation(int SessionindexToJoin) override;
	// IMainMenuInterface
	//////////////////////

private:
	IOnlineSessionPtr SessionInterface;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	FName SessionName = TEXT("Gwang Session Game");
	FName ServerNameSettingsKey = TEXT("ServerName");
	FString ServerName = "NO_NAME";

	UMainMenuWidget* MainMenu;

private:
	void CreateSession();
	void OnCreateSessionComplete(FName Name, bool bSuccess);
	void OnDestroySessionComplete(FName Name, bool bSuccess);
	void OnFindSessionComplete(bool bSuccess);
	void OnJoinSessionComplete(FName Name, EOnJoinSessionCompleteResult::Type Type);
public:
};

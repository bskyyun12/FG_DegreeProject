// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSubSystem.h"
#include "Widgets/MainMenu/MainMenuInterface.h"
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

	void LoadMainMenu(TSubclassOf<UUserWidget> MainMenuWidgetClass, TSubclassOf<UUserWidget> SessionInfoRowClass);

	//////////////////////
	// IMainMenuInterface
	void Host_Implementation() override;
	void Find_Implementation() override;
	// IMainMenuInterface
	//////////////////////

private:
	IOnlineSessionPtr SessionInterface;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	FName SessionName = TEXT("Gwang Session Game");

	UMainMenuWidget* MainMenu;

private:
	void CreateSession();
	void OnCreateSessionComplete(FName Name, bool bSuccess);
	void OnDestroySessionComplete(FName Name, bool bSuccess);
	void OnFindSessionComplete(bool bSuccess);
};

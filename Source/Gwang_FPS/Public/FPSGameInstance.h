// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSubSystem.h"
#include "Widgets/MainMenuInterface.h"
#include "FPSGameInstance.generated.h"

class UUserWidget;
class UMainMenuWidget;

UCLASS()
class GWANG_FPS_API UFPSGameInstance : public UGameInstance, public IMainMenuInterface
{
	GENERATED_BODY()
	
public:
	UFPSGameInstance(const FObjectInitializer& ObjectIn);

	void Init() override;

	UFUNCTION(BlueprintCallable)
	void LoadMenuwidget();

	UFUNCTION(Exec)
	void JoinWithIP(const FString& IPAddress);

	void SetMainMenuWidget(TSubclassOf<UUserWidget> MainMenuClass);

	//////////////////////
	// IMainMenuInterface

	void Host_Implementation() override;

	// IMainMenuInterface
	//////////////////////

private:
	IOnlineSessionPtr SessionInterface;

	FName SessionName = TEXT("Gwang Session Game");

	TSubclassOf<UUserWidget> MainMenuWidgetClass;
	UMainMenuWidget* MainMenu;

private:
	void CreateSession();
	void OnCreateSessionComplete(FName Name, bool bSuccess);
	void OnDestroySessionComplete(FName Name, bool bSuccess);
};

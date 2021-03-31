// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "OnlineSubSystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MainMenu/MenuWidgets/MainMenuInterface.h"

#include "FPSGameInstance.generated.h"

class UUserWidget;
class UMainMenuWidget;
class FOnlineSessionSearch;

UENUM(BlueprintType)
enum class ETeam : uint8 {
	None, 
	Marvel,
	DC,
};

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

	void DestroySession();

	void SetTeam(ETeam InTeam);
	ETeam GetTeam();

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

	ETeam Team = ETeam::None;

private:
	void CreateSession();
	void OnCreateSessionComplete(FName Name, bool bSuccess);
	void OnDestroySessionComplete(FName Name, bool bSuccess);
	void OnFindSessionComplete(bool bSuccess);
	void OnJoinSessionComplete(FName Name, EOnJoinSessionCompleteResult::Type Type);

	// Called on NetworkFailure
	void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type Type, const FString& ErrorMsg);
public:
};

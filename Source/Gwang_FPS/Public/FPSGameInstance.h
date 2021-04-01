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
class AFPSWeaponBase;

UENUM(BlueprintType)
enum class ETeam : uint8 {
	None, 
	Marvel,
	DC,
};

UENUM(BlueprintType)
enum class EMainWeapon : uint8
{
	None,
	Rifle,
	Shotgun
};

UENUM(BlueprintType)
enum class ESubWeapon : uint8
{
	None,
	Pistol,
};

USTRUCT(BlueprintType)
struct FUserData
{
	GENERATED_BODY()

	UPROPERTY()
	FName UserName;

	UPROPERTY()
	int ControllerID;

	UPROPERTY()
	bool bIsReady;

	UPROPERTY()
	ETeam Team;

	UPROPERTY()
	EMainWeapon MainWeaponType;

	UPROPERTY()
	ESubWeapon SubWeaponType;

	FUserData()
	{
		UserName = "Gwang";
		ControllerID = 0;
		bIsReady = false;
		Team = ETeam::None;

		MainWeaponType = EMainWeapon::None;
		SubWeaponType = ESubWeapon::None;
	}

	bool operator == (FUserData const& UserData)
	{
		return ControllerID == UserData.ControllerID;
	}
};

UCLASS()
class GWANG_FPS_API UFPSGameInstance : public UGameInstance, public IMainMenuInterface
{
	GENERATED_BODY()
	
public:
	UFPSGameInstance(const FObjectInitializer& ObjectIn);

	void Init() override;

	FUserData GetUserData() const;
	void SetUserData(const FUserData& Data);

	void SetMainMenu(UMainMenuWidget* InMainMenu);
	void LoadMainMenu(TSubclassOf<UUserWidget> MainMenuWidgetClass, TSubclassOf<UUserWidget> SessionInfoRowClass);

	void StartSession();
	void DestroySession();

	// IMainMenuInterface
	void Host_Implementation(const FString& InServerName) override;
	void Find_Implementation() override;
	void Join_Implementation(int SessionindexToJoin) override;

private:
	IOnlineSessionPtr SessionInterface;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	FName SessionName = TEXT("Gwang Session Game");
	FName ServerNameSettingsKey = TEXT("ServerName");
	FString ServerName = "NO_NAME";

	UMainMenuWidget* MainMenu;
	FUserData UserData;

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

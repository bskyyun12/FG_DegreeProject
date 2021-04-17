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

// Each enum value represents index of weapon array in FPSCharacter
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	None,
	MainWeapon,
	SubWeapon,
	Melee,
	Grenade,

	EnumSize,	// Leave the enum value as default
};

UENUM(BlueprintType)
enum class EMainWeapon : uint8	// NAME CHANGE? Make sure to change the name in ULobbyInventory::Initialize!
{
	M4A1,
	AK47,
	EnumSize
};

UENUM(BlueprintType)
enum class ESubWeapon : uint8	// NAME CHANGE? Make sure to change the name in ULobbyInventory::Initialize!
{
	Pistol,
	EnumSize
};

UENUM(BlueprintType)
enum class EMeleeWeapon : uint8	// NAME CHANGE? Make sure to change the name in ULobbyInventory::Initialize!
{
	Knife,
	EnumSize
};

UENUM(BlueprintType)
enum class EGrenade : uint8	// NAME CHANGE? Make sure to change the name in ULobbyInventory::Initialize!
{
	Grenade,
	EnumSize
};

UENUM(BlueprintType)
enum class ETeam : uint8 {
	None, 
	Marvel,
	DC,
	EnumSize
};

USTRUCT(BlueprintType)
struct FPlayerData
{
	GENERATED_BODY()

	UPROPERTY()
	FName PlayerName;

	UPROPERTY()
	ETeam Team;

	UPROPERTY()
	EMainWeapon StartMainWeapon;

	UPROPERTY()
	ESubWeapon StartSubWeapon;

	UPROPERTY()
	EMeleeWeapon StartMeleeWeapon;

	UPROPERTY()
	EGrenade StartGrenade;

	FPlayerData()
	{
		PlayerName = "Gwang";
		Team = ETeam::None;

		StartMainWeapon = EMainWeapon::M4A1;
		StartSubWeapon = ESubWeapon::Pistol;
		StartMeleeWeapon = EMeleeWeapon::Knife;
		StartGrenade = EGrenade::Grenade;
	}
};

UCLASS()
class GWANG_FPS_API UFPSGameInstance : public UGameInstance, public IMainMenuInterface
{
	GENERATED_BODY()
	
public:
	UFPSGameInstance(const FObjectInitializer& ObjectIn);

	void Init() override;

	FPlayerData GetPlayerData() const;
	void SetPlayerData(const FPlayerData& Data);

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
	FPlayerData PlayerData;

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

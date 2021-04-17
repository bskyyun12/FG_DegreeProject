// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FPSGameInstance.h"
#include "LobbyGameMode.generated.h"

class ALobbyPlayerController;

USTRUCT(BlueprintType)
struct FLobbyPlayerData
{
	GENERATED_BODY()

	UPROPERTY()
	FName PlayerName;

	UPROPERTY()
	ETeam Team;

	UPROPERTY()
	bool bIsReady;

	UPROPERTY()
	EMainWeapon StartMainWeapon;

	UPROPERTY()
	ESubWeapon StartSubWeapon;

	UPROPERTY()
	EMeleeWeapon StartMeleeWeapon;

	UPROPERTY()
	EGrenade StartGrenade;

	UPROPERTY()
	APlayerController* PlayerController;

	UPROPERTY()
	bool bFinishedSavingData;

	FLobbyPlayerData()
	{
		PlayerName = "Gwang";
		Team = ETeam::None;
		bIsReady = false;

		StartMainWeapon = EMainWeapon::M4A1;
		StartSubWeapon = ESubWeapon::Pistol;
		StartMeleeWeapon = EMeleeWeapon::Knife;
		StartGrenade = EGrenade::Grenade;

		PlayerController = nullptr;

		bFinishedSavingData = false;
	}
};

UCLASS()
class GWANG_FPS_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	// Getters
	FLobbyPlayerData GetLobbyPlayerData(APlayerController* PlayerController) const;

	void UpdateLobbyPlayerData(const FLobbyPlayerData& UpdatedData);

	void UpdateLobbyUI();

	void RemoveLobbyPlayerData(APlayerController* PlayerController);

protected:
	TArray<FLobbyPlayerData> LobbyPlayerData;
	TArray<APlayerController*> PlayerControllers;

	FTimerHandle GameStartTimer;

	bool bAllPlayerReady;
protected:
	// Getters
	ETeam GetTeamToJoin();
	FName GetUserName(APlayerController* NewPlayer) const;

	void PostLogin(APlayerController* NewPlayer) override;
	void Logout(AController* Exiting) override;

	bool IsReadyToStartGame();

	void StartGame();

	UFUNCTION()
	void GameStartCheck();
};

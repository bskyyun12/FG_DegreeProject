// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerControllerInterface.h"
#include "DeathMatchPlayerController.generated.h"

class ADeathMatchGameMode;
class ADeathMatchCharacter;
class ADeathMatchPlayerState;
class UFPSHUDWidget;
class UVignetteWidget;
class UScoreBoardWidget;
class UGameOverWidget;

UCLASS()
class GWANG_FPS_API ADeathMatchPlayerController : public APlayerController, public IPlayerControllerInterface
{
	GENERATED_BODY()

public:	
	// Called after ADeathMatchGameMode::PostLogin
	UFUNCTION(Server, Reliable)
	void Server_OnPostLogin();

	// Called after ADeathMatchGameMode::SpawnPlayer
	UFUNCTION(Server, Reliable)
	void Server_OnSpawnPlayer(ADeathMatchCharacter* SpawnedPlayer);
	UFUNCTION(Client, Reliable)
	void Client_OnSpawnPlayer();

	void OnPlayerDeath_Implementation() override;
	UFUNCTION(Server, Reliable)
	void Server_OnPlayerDeath();
	UFUNCTION(Client, Reliable)
	void Client_OnPlayerDeath();

#pragma region Widget Related
	UFUNCTION(Client, Reliable)
	void Client_SetupWidgets();

	void UpdateMatchTimeUI(const float& MatchTime);
	void UpdateScoreUI(const uint8& MarvelScore, const uint8& DCScore);

	// Crosshair
	void ChangeCrosshairUIOnHit_Implementation() override;

	// TakeDamage
	void VignetteEffectOnTakeDamage_Implementation() override;
	void UpdateHealthArmorUI(const uint8& CurrentHealth, const uint8& CurrentArmor);

	// Chat
	void StartChat_Implementation() override;
	void SendChat(const FName& ChatContent);
	void UpdateChatUI(const FName& PlayerName, const FName& ChatContent);
	
	void UpdateWeaponUI_Implementation(const FName& WeaponName, const int& CurrentAmmo, const int& RemainingAmmo) override;
	void LoadGameOverUI(const bool& bIsWinner, const bool& bWidgetVisibility);
	void SetScoreBoardUIVisibility_Implementation(bool bNewVisibility) override;
#pragma endregion Widget Related

protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	ADeathMatchGameMode* GM;
	ADeathMatchPlayerState* PS;

#pragma region Widget Related

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> HUDWidgetClass;
	UFPSHUDWidget* HUDWidget;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> VignetteWidgetClass;
	UVignetteWidget* VignetteWidget;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> ScoreboardWidgetClass;
	UScoreBoardWidget* ScoreboardWidget;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> GameOverWidgetClass;
	UGameOverWidget* GameOverWidget;

#pragma endregion Widget Related

	FTimerHandle ReadyCheckTimer;
protected:
	
	UFUNCTION(Client, Reliable)
	void Client_CheckReadyStatus();
	UFUNCTION(Server, Reliable)
	void Server_RequestPlayerSpawn();

	bool bWidgetLoaded;
public:





};

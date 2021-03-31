// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPSPlayerControllerInterface.h"
#include "Widgets/FPSWidgetBase.h"
#include "FPSPlayerController.generated.h"

class UUserWidget;
class AFPSGameMode;
class AFPSGameState;
class UFPSHUDWidget;
class UGameOverWidget;
class UScoreBoardWidget;
class UDamageReceiveWidget;

UCLASS()
class GWANG_FPS_API AFPSPlayerController : public APlayerController, public IFPSPlayerControllerInterface
{
	GENERATED_BODY()
	
public:
	////////////////////////////////
	// IFPSPlayerControllerInterface

	// Getters
	ETeam GetTeam_Implementation() override;

	// PostLogin Setup
	void OnPostLogin_Implementation(AFPSGameMode* FPSGameMode) override;
	UFUNCTION(Client, Reliable)
	void Client_SetupTeam();
	UFUNCTION(Server, Reliable)
	void Server_SetupTeam(ETeam TeamFromGameInstance);

	// Spawn & Death
	UFUNCTION(Server, Reliable)
	void Server_RequestPlayerSpawn();
	void OnSpawnPlayer_Implementation(AFPSCharacter* SpawnedPlayer) override;
	void OnPlayerDeath_Implementation() override;

	// Hit & Damage
	void OnApplyDamage_Implementation() override;
	UFUNCTION(Client, Reliable)
	void Client_OnApplyDamage();
	void OnTakeDamage_Implementation() override;
	UFUNCTION(Client, Reliable)
	void Client_OnTakeDamage();

	// Widgets
	void ToggleScoreBoardWidget_Implementation(bool bVisible) override;
	void OnUpdateHealthArmorUI_Implementation(bool bIsDead) override;
	UFUNCTION(Client, Reliable)
	void Client_OnUpdateHealthArmorUI(bool bIsDead);
	void OnUpdateAmmoUI_Implementation(int CurrentAmmo, int RemainingAmmo) override;
	UFUNCTION(Client, Reliable)
	void Client_OnUpdateAmmoUI(int CurrentAmmo, int RemainingAmmo);

	// Others
	void ShakeCamera_Implementation(TSubclassOf<UCameraShakeBase> CameraShake) override;
	void AddControlRotation_Implementation(const FRotator& RotationToAdd) override;

	// IFPSPlayerControllerInterface
	////////////////////////////////

protected:
	// Widgets
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> FPSHUDWidgetClass;
	UFPSHUDWidget* FPSHUDWidget;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> GameOverWidgetClass;
	UGameOverWidget* GameOverWidget;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> ScoreBoardWidgetClass;
	UScoreBoardWidget* ScoreBoardWidget;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> DamageReceiveWidgetClass;
	UDamageReceiveWidget* DamageReceiveWidget;

	// Non widgets :)
	UPROPERTY()
	AFPSGameMode* GameMode;

	UPROPERTY()
	AFPSGameState* GameState;

	UPROPERTY(Replicated)
	ETeam Team;

	FTimerHandle DamageReceiveTimer;

protected:
	void BeginPlay() override;
	UFUNCTION(Client, Reliable)
	void Client_SetupWidgets();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// GameMode Delegate Listeners	
	void GameModeSetup(AFPSGameMode* FPSGameMode);
	void GameStateSetup();

	UFUNCTION()
	void OnStartMatch();	// Bound to GameMode->OnStartMatch		

	UFUNCTION()
	void OnEndMatch();	// Bound to GameMode->OnEndMatch
	UFUNCTION(Client, Reliable)
	void Client_LoadGameOver(bool bIsWinner, bool bWidgetVisibility);
	
	UFUNCTION()
	void OnUpdateScoreUI(int MarvelScore, int DCScore);	// Bound to GameState->OnUpdateScoreUI
	UFUNCTION(Client, Reliable)
	void Client_OnUpdateScoreUI(int MarvelScore, int DCScore);
};

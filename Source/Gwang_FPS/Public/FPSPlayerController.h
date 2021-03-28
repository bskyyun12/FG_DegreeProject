// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "FPSPlayerControllerInterface.h"
#include "Widgets/FPSWidgetBase.h"

#include "FPSPlayerController.generated.h"

class UUserWidget;
class AFPSGameMode;
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
	void StartNewGame_Implementation() override;
	UFUNCTION(Client, Reliable)
	void Client_StartNewGame();
	UFUNCTION(Server, Reliable)
	void Server_StartNewGame(ETeam InTeam);

	void OnSpawnPlayer_Implementation(AFPSCharacter* PooledPlayer) override;

	void RespawnPlayer_Implementation() override;

	void OnPlayerDeath_Implementation() override;
	UFUNCTION(Server, Reliable)
	void Server_OnPlayerDeath();

	void ShakeCamera_Implementation(TSubclassOf<UCameraShakeBase> CameraShake) override;

	void AddControlRotation_Implementation(const FRotator& RotationToAdd) override;

	void OnTakeDamage_Implementation() override;

	void ToggleScoreBoardWidget_Implementation(bool bVisible) override;

	void LoadGameOverWidget_Implementation(ETeam WinnerTeam) override;
	UFUNCTION(Client, Reliable)
	void Client_LoadGameOver(ETeam WinnerTeam);

	void OnApplyDamage_Implementation() override;
	UFUNCTION(Client, Reliable)
	void Client_OnApplyDamage();

	void OnUpdateHealthArmorUI_Implementation(bool bIsDead) override;
	UFUNCTION(Client, Reliable)
	void Client_OnUpdateHealthArmorUI(bool bIsDead);

	// IFPSPlayerControllerInterface
	////////////////////////////////

protected:
	///////////
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
	// Widgets
	///////////

	UPROPERTY()
	AFPSGameMode* GameMode;

	UPROPERTY(Replicated)
	ETeam Team;

protected:
	void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Client, Reliable)
	void Client_SetupWidgets();

private:
	FTimerHandle DamageReceiveTimer;

};

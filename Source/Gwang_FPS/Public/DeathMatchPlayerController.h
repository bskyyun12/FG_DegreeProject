// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerControllerInterface.h"
#include "DeathMatchPlayerController.generated.h"

class ADeathMatchGameMode;
class ADeathMatchPlayerState;
class UFPSHUDWidget;
class UDamageReceiveWidget;
class UScoreBoardWidget;
class UGameOverWidget;

UCLASS()
class GWANG_FPS_API ADeathMatchPlayerController : public APlayerController, public IPlayerControllerInterface
{
	GENERATED_BODY()

public:	
	
#pragma region Widget Related
	UFUNCTION(Client, Reliable)
	void Client_SetupWidgets();

	void UpdateMatchTimeUI(const float& MatchTime);
	void UpdateScoreUI(const uint8& MarvelScore, const uint8& DCScore);
	void UpdateCrosshairUIOnHit();
	void VignetteUIOnTakeDamage();

	void SendChat(const FName& PlayerName, const FName& ChatContent);
	void UpdateChatUI(const FName& PlayerName, const FName& ChatContent);

	// IPlayerControllerInterface
	void UpdateAmmoUI_Implementation(const int& CurrentAmmo, const int& RemainingAmmo) override;
	void LoadGameOverUI(const bool& bIsWinner, const bool& bWidgetVisibility);
	void SetScoreBoardUIVisibility_Implementation(bool bNewVisibility) override;
	void OnUpdateHealthArmorUI_Implementation(const uint8& CurrentHealth, const uint8& CurrentArmor) override;
	void StartChat_Implementation() override;
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
	UDamageReceiveWidget* VignetteWidget;
	FTimerHandle VignetteTimer;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> ScoreboardWidgetClass;
	UScoreBoardWidget* ScoreboardWidget;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> GameOverWidgetClass;
	UGameOverWidget* GameOverWidget;

#pragma endregion Widget Related

protected:
	void BeginPlay() override;

	void OnPossess(APawn* aPawn) override;
	UFUNCTION(Client, Reliable)
	void Client_OnPossess();

public:







};

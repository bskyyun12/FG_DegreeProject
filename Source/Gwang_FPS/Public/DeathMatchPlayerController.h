// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DeathMatchPlayerController.generated.h"

class ADeathMatchGameMode;
class ADeathMatchPlayerState;
class UFPSHUDWidget;
class UDamageReceiveWidget;
class UScoreBoardWidget;

UCLASS()
class GWANG_FPS_API ADeathMatchPlayerController : public APlayerController
{
	GENERATED_BODY()

public:	
	
#pragma region Widget Related

	void UpdateMatchTimeUI(const float& MatchTime);
	void UpdateAmmoUI(const uint16& CurrentAmmo, const uint16& RemainingAmmo);
	void UpdateHealthArmorUI(const uint8& Health, const uint8& Armor);
	void UpdateScoreUI(const uint8& MarvelScore, const uint8& DCScore);
	void UpdateCrosshairUIOnHit();
	void VignetteUIOnTakeDamage();
	void SetScoreBoardUIVisibility(const bool& NewVisibility);

	void StartChat();
	void SendChat(const FName& PlayerName, const FName& ChatContent);
	void UpdateChatUI(const FName& PlayerName, const FName& ChatContent);

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
#pragma endregion Widget Related

protected:
	void BeginPlay() override;

	void OnPossess(APawn* aPawn) override;
	UFUNCTION(Client, Reliable)
	void Client_OnPossess();

};

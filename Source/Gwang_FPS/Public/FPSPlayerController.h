// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "FPSPlayerControllerInterface.h"

#include "FPSPlayerController.generated.h"

class UUserWidget;
class AFPSGameMode;
class UTeamSelectionWidget;
class UGameOverWidget;
class UGameStatusWidget;

UCLASS()
class GWANG_FPS_API AFPSPlayerController : public APlayerController, public IFPSPlayerControllerInterface
{
	GENERATED_BODY()
	
public:	
	////////////////////////////////
	// IFPSPlayerControllerInterface
	void StartNewGame_Implementation() override;
	UFUNCTION(Server, Reliable)
	void Server_StartNewGame();

	UFUNCTION(Client, Reliable)
	void Client_LoadTeamSelection();

	void OnTeamSelected_Implementation(ETeam InTeam) override;
	UFUNCTION(Server, Reliable)
	void Server_OnTeamSelected(ETeam InTeam);

	void OnSpawnPlayer_Implementation(AFPSCharacter* PooledPlayer) override;

	void RespawnPlayer_Implementation() override;

	void OnPlayerDeath_Implementation() override;

	void LoadGameOver_Implementation(bool Victory) override;
	UFUNCTION(Client, Reliable)
	void Client_LoadGameOver(bool Victory);

	void ShakeCamera_Implementation(TSubclassOf<UCameraShakeBase> CameraShake) override;

	void AddControlRotation_Implementation(const FRotator& RotationToAdd) override;

	ETeam GetTeam_Implementation() override;

	void HandleGameStatusWidget_Implementation(bool bDisplay) override;
	// IFPSPlayerControllerInterface
	////////////////////////////////

protected:
	///////////
	// Widgets
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> TeamSelectionClass;
	UTeamSelectionWidget* TeamSelection;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> GameOverWidgetClass;
	UGameOverWidget* GameOverWidget;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> GameStatusWidgetClass;
	UGameStatusWidget* GameStatusWidget;
	// Widgets
	///////////

	UPROPERTY()
	AFPSGameMode* GameMode;

	ETeam Team;

protected:
	void BeginPlay() override;

	UFUNCTION()
	void OnUpdateTeamSelectionUI(ETeam InTeam, bool bCanJoinTeam);
	UFUNCTION(Client, Reliable)
	void Client_OnUpdateTeamSelectionUI(ETeam InTeam, bool bCanJoinTeam);

	UFUNCTION()
	void OnEndGame(ETeam WinnerTeam);
public:



};

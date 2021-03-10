// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPSPlayerControllerInterface.h"
#include "FPSPlayerController.generated.h"


class UUserWidget;
class UTeamSelectionWidget;
class AFPSGameMode;

UCLASS()
class GWANG_FPS_API AFPSPlayerController : public APlayerController, public IFPSPlayerControllerInterface
{
	GENERATED_BODY()
	
public:
	AFPSPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void BeginPlay() override;

#pragma region IFPSPlayerControllerInterface
	virtual void LoadTeamSelection_Implementation() override;
	UFUNCTION(Client, Reliable)
	void Client_LoadTeamSelection(TSubclassOf<UUserWidget> teamSelectionClass);

	void OnTeamSelected_Implementation(ETeam InTeam) override;
	UFUNCTION(Server, Reliable)
	void Server_OnTeamSelected(ETeam InTeam);

	void OnSpawnPlayer_Implementation(TSubclassOf<AFPSCharacter> CharacterClass) override;

	void RespawnPlayer_Implementation() override;
#pragma endregion

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> TeamSelectionClass;

	UTeamSelectionWidget* TeamSelection;

	AFPSGameMode* GameMode;

	ETeam Team;

protected:
};

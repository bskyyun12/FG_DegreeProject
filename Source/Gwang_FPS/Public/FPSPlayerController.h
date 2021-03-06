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

	// IFPSPlayerControllerInterface
	virtual void LoadTeamSelection_Implementation() override;
	UFUNCTION(Client, Reliable)
	void Client_LoadTeamSelection(TSubclassOf<UUserWidget> teamSelectionClass);

	void OnDarkTeamSelected_Implementation() override;
	UFUNCTION(Server, Reliable)
	void Server_OnDarkTeamSelected();

	void OnSilverTeamSelected_Implementation() override;
	UFUNCTION(Server, Reliable)
	void Server_OnSilverTeamSelected();

	void OnSpawnPlayer_Implementation(TSubclassOf<AFPSCharacter> CharacterClass, FTransform Transform) override;

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> TeamSelectionClass;

	UTeamSelectionWidget* TeamSelection;

	AFPSGameMode* GameMode;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FPSGameInstance.h"
#include "DeathMatchGameMode.generated.h"

class ADeathMatchCharacter;
class ADeathMatchPlayerController;

UCLASS()
class GWANG_FPS_API ADeathMatchGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	TArray<ADeathMatchPlayerController*> PlayerControllers;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ADeathMatchCharacter> Marvel_CharacterClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ADeathMatchCharacter> DC_CharacterClass;

protected:
	// Called after a successful login
	void PostLogin(APlayerController* NewPlayer) override;

	// Called when a player leaves the game or is destroyed.
	void Logout(AController* Exiting) override;

	void BeginPlay() override;

	UFUNCTION(Exec)
	void SpawnPlayers();
	ETeam GetTeamWithLessPlayers();
	AActor* GetBestPlayerStart(const FString& PlayerStartTag) const;


public:



};

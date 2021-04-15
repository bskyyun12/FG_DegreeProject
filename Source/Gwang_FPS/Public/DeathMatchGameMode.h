// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FPSGameInstance.h"
#include "DeathMatchGameMode.generated.h"

class ADeathMatchGameState;
class ADeathMatchCharacter;
class ADeathMatchPlayerController;

USTRUCT(BlueprintType)
struct FWeaponClass
{
	GENERATED_BODY()

	// Main Weapons
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> M4A1Class;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> AK47Class;

	// Sub Weapons
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> PistolClass;

	// Knives
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> KnifeClass;

	// Grenades
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> GrenadeClass;

};

UCLASS()
class GWANG_FPS_API ADeathMatchGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	float GetMatchTimeInSeconds() const { return MatchTimeInSeconds; }

	// Finish the game and move players to lobby
	void EndMatch();

	FWeaponClass GetWeaponClass() const { return WeaponClass; }

	// Spawn Player
	void SpawnPlayer(ADeathMatchPlayerController* PC);

	// Handle Player Death
	void OnPlayerDeath(ADeathMatchPlayerController* PC);

protected:
	ADeathMatchGameState* GS;

	TArray<ADeathMatchPlayerController*> PlayerControllers;

	UPROPERTY(EditDefaultsOnly)
	FWeaponClass WeaponClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ADeathMatchCharacter> Marvel_CharacterClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ADeathMatchCharacter> DC_CharacterClass;

	UPROPERTY(EditDefaultsOnly)
	float MatchTimeInSeconds = 10.f; // TODO: this should be 600.f if the match should last for 10 mins for example

	UPROPERTY(EditDefaultsOnly)
	float RespawnDelay = 5.f;

	FTimerHandle MatchStartTimer;
protected:

	void InitGameState() override;

	// Called after a successful login
	void PostLogin(APlayerController* NewPlayer) override;

	// Called when a player leaves the game or is destroyed.
	void Logout(AController* Exiting) override;

	// Spawn players and start the game!
	UFUNCTION(Exec)
	void RestartMatch();
		
	// Spawn Player
	ETeam GetTeamWithLessPeople();
	AActor* GetBestPlayerStart(const FString& PlayerStartTag) const;
};

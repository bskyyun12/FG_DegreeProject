// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "FPSGameInstance.h"
#include "DeathMatchPlayerState.generated.h"

class UFPSGameInstance;
class ADeathMatchCharacter;
class ADeathMatchPlayerController;

UCLASS()
class GWANG_FPS_API ADeathMatchPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	// Getters
	ETeam GetTeam() const { return Team; }
	uint8 GetCurrentHealth() const { return CurrentHealth; }
	uint8 GetCurrentArmor() const { return CurrentArmor; }
	TArray<AActor*> GetCurrentWeapons() const { return CurrentWeapons; }
	AActor* GetCurrentWeaponWithIndex(const uint8& Index) const;
	AActor* GetCurrentlyHeldWeapon() const { return CurrentlyHeldWeapon; }
	bool GetIsReadyToJoin() const { return bIsReadyToJoin; }
	bool GetIsDead() const { return bIsDead; }
	ADeathMatchPlayerController* GetPlayerController();

	// Called after ADeathMatchGameMode::PostLogin
	UFUNCTION(Server, Reliable)
	void Server_OnPostLogin();

	void SetCurrentlyHeldWeapon(AActor* NewWeapon) { CurrentlyHeldWeapon = NewWeapon; }
	void SetCurrentWeaponWithIndex(const uint8& Index, AActor* NewWeapon) { CurrentWeapons[Index] = NewWeapon; }

	UFUNCTION(Server, Reliable)
	void Server_SetTeam(const ETeam& NewTeam);

	UFUNCTION(Server, Reliable)
	void Server_OnKillPlayer();

	UFUNCTION(Server, Reliable)
	void Server_OnDeath();

	UFUNCTION(Server, Reliable)
	void Server_UpdateMatchTimeLeft(const float& TimeLeft);

	UFUNCTION(Server, Reliable)
	void Server_OnSendChat(const FName& ChatContent);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnSendChat(const FName& ChatContent);

	UFUNCTION(Server, Reliable)
	void Server_UpdateHealthArmor(const uint8& NewHealth, const uint8& NewArmor);

	UFUNCTION(Server, Reliable)
	void Server_OnSpawn();

protected:
	ADeathMatchPlayerController* PC;
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsReadyToJoin;

	UPROPERTY(Replicated, BlueprintReadOnly)
	ETeam Team;

	UPROPERTY(Replicated, BlueprintReadOnly)
	TArray<TSubclassOf<AActor>> StartWeaponClasses;
	UPROPERTY(Replicated, BlueprintReadOnly)
	TArray<AActor*> CurrentWeapons;
	UPROPERTY(Replicated, BlueprintReadOnly)
	AActor* CurrentlyHeldWeapon;

	UPROPERTY(Replicated, BlueprintReadOnly)
	uint8 CurrentHealth;
	UPROPERTY(Replicated, BlueprintReadOnly)
	uint8 CurrentArmor;

	UPROPERTY(Replicated)
	uint8 NumKills;

	UPROPERTY(Replicated)
	uint8 NumDeaths;

	UPROPERTY(Replicated)
	bool bIsDead;

	UPROPERTY(Replicated)
	float MatchTimeLeft = 0.f;

protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Called after Server_OnPostLogin
	UFUNCTION(Client, Reliable)
	void Client_OnPostLogin();

	// Called after Client_OnPostLogin
	UFUNCTION(Server, Reliable)
	void Server_InitialDataSetup(const FPlayerData& PlayerData);

	UFUNCTION(Client, Reliable)
	void Client_UpdateHealthUI(const uint8& NewHealth, const uint8& NewArmor);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnSpawn();
	
	UFUNCTION(Server, Reliable)
	void Server_ResetCurrentWeapons();

};

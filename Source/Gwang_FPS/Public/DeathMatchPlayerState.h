// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "FPSGameInstance.h"
#include "DeathMatchPlayerState.generated.h"

class ADeathMatchGameMode;
class UFPSGameInstance;
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
	AActor* GetStartMainWeapon() const { return StartWeapons[1]; }
	AActor* GetCurrentMainWeapon() const { return CurrentWeapons[1]; }
	AActor* GetCurrentSubWeapon() const { return CurrentWeapons[2]; }
	AActor* GetCurrentWeaponWithIndex(const uint8& Index) const;
		
	void OnPostLogin();

	UFUNCTION(Server, Reliable)
	void Server_UpdateCurrentWeapons(const uint8& Index, AActor* NewWeapon);

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

	UFUNCTION(Server, Reliable)
	void Server_UpdateHealthArmor(const uint8& NewHealth, const uint8& NewArmor);


protected:
	ADeathMatchGameMode* GM;
	ADeathMatchPlayerController* PC;
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	FName PlayerName;
	UPROPERTY(Replicated, BlueprintReadOnly)
	ETeam Team;

	UPROPERTY(Replicated, BlueprintReadOnly)
	TArray<AActor*> StartWeapons;
	UPROPERTY(Replicated, BlueprintReadOnly)
	TArray<AActor*> CurrentWeapons;

	UPROPERTY(Replicated, BlueprintReadOnly)
	uint8 CurrentHealth;
	UPROPERTY(Replicated, BlueprintReadOnly)
	uint8 CurrentArmor;

	UPROPERTY(ReplicatedUsing = OnRep_NumKills)
	uint8 NumKills;
	UFUNCTION()
	void OnRep_NumKills();

	UPROPERTY(ReplicatedUsing = OnRep_NumDeaths)
	uint8 NumDeaths;
	UFUNCTION()
	void OnRep_NumDeaths();

	UPROPERTY(ReplicatedUsing = OnRep_bIsDead)
	bool bIsDead;
	UFUNCTION()
	void OnRep_bIsDead();

	UPROPERTY(ReplicatedUsing=OnRep_MatchTimeLeft)
	float MatchTimeLeft = 0.f;
	UFUNCTION()
	void OnRep_MatchTimeLeft();
	
	UPROPERTY(ReplicatedUsing=OnRep_LastChat)
	FName LastChat;
	UFUNCTION()
	void OnRep_LastChat();

protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void PostInitializeComponents() override;

	// Called after PostLogin
	UFUNCTION(Client, Reliable)
	void Client_ReadData();
	UFUNCTION(Server, Reliable)
	void Server_ReceiveData(const FPlayerData& PlayerData);

	// bound to ADeathMatchGameMode::OnStartMatch delegate
	UFUNCTION(Server, Reliable)
	void Server_OnStartMatch();
	UFUNCTION(Client, Reliable)
	void Client_UpdateHealthUI(const uint8& NewHealth, const uint8& NewArmor);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnStartMatch();

};

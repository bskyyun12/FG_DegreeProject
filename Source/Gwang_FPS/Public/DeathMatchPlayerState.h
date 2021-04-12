// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "FPSGameInstance.h"
#include "DeathMatchPlayerState.generated.h"

class ADeathMatchGameMode;
class UFPSGameInstance;
class ADeathMatchPlayerController;

USTRUCT(BlueprintType)
struct FPlayerHealth
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 CurrentArmor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 MaxArmor;

	FPlayerHealth()
	{
		MaxHealth = 100;
		MaxArmor = 100;
	}
};

USTRUCT(BlueprintType)
struct FPlayerInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Ping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETeam Team;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 NumKills;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 NumDeaths;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDead;
};

USTRUCT(BlueprintType)
struct FChat
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ChatContent;
};

UCLASS()
class GWANG_FPS_API ADeathMatchPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	UPROPERTY(ReplicatedUsing=OnRep_PlayerHealth)
	FPlayerHealth PlayerHealth;
	UFUNCTION()
	void OnRep_PlayerHealth();

	UFUNCTION(Server, Reliable)
	void Server_UpdatePlayerHealth(const uint8& Health);

public:
	FPlayerInfo GetPlayerInfo() const { return PlayerInfo; }
	
	UFUNCTION(Server, Reliable)
	void Server_OnSpawn();

	UFUNCTION(Server, Reliable)
	void Server_SetTeam(const ETeam& NewTeam);

	UFUNCTION(Server, Reliable)
	void Server_OnKillPlayer();

	UFUNCTION(Server, Reliable)
	void Server_OnDeath();

	UFUNCTION(Server, Reliable)
	void Server_UpdateMatchTimeLeft(const float& TimeLeft);

	UFUNCTION(Server, Reliable)
	void Server_OnSendChat(const FName& PlayerName, const FName& ChatContent);

protected:
	ADeathMatchGameMode* GM;
	UFPSGameInstance* GI;
	ADeathMatchPlayerController* PC;

	FPlayerData PlayerData;

	UPROPERTY(ReplicatedUsing=OnRep_PlayerInfo)
	FPlayerInfo PlayerInfo;
	UFUNCTION()
	void OnRep_PlayerInfo();

	UPROPERTY(ReplicatedUsing=OnRep_MatchTimeLeft)
	float MatchTimeLeft = 0.f;
	UFUNCTION()
	void OnRep_MatchTimeLeft();
	
	UPROPERTY(ReplicatedUsing=OnRep_LastChat)
	FChat LastChat;
	UFUNCTION()
	void OnRep_LastChat();

protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void PostInitializeComponents() override;

	void BeginPlay() override;

};

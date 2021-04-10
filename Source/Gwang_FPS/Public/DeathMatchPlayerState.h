// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "FPSGameInstance.h"
#include "DeathMatchPlayerState.generated.h"

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
};

UCLASS()
class GWANG_FPS_API ADeathMatchPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	FPlayerInfo GetPlayerInfo() const { return PlayerInfo; }

	UFUNCTION(Server, Reliable)
	void Server_SetTeam(const ETeam& NewTeam);

	UFUNCTION(Server, Reliable)
	void Server_AddNumKill();

	UFUNCTION(Server, Reliable)
	void Server_AddNumDeath();

protected:
	UPROPERTY(ReplicatedUsing= OnRep_PlayerInfo)
	FPlayerInfo PlayerInfo;

	UFUNCTION()
	void OnRep_PlayerInfo();

protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void PostInitializeComponents() override;

};

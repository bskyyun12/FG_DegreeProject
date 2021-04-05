// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/FPSWeaponBase.h"
#include "FPSGrenadeBase.generated.h"

USTRUCT(BlueprintType)
struct FGrenadeMove
{
	GENERATED_BODY()

	UPROPERTY()
	FVector NewPoint;;
	UPROPERTY()
	FVector LaunchPoint;
	UPROPERTY()
	FVector PrevPoint;
	UPROPERTY()
	FVector LaunchForward;
	UPROPERTY()
	FVector LaunchUp;
	UPROPERTY()
	float CurrentSpeed;	
	UPROPERTY()
	float LaunchAngleInRad;
	UPROPERTY()
	float FlightTime;

	UPROPERTY()
	float DeltaTime;
	UPROPERTY()
	float Time;
};

USTRUCT(BlueprintType)
struct FGrenadeState
{
	GENERATED_BODY()

	UPROPERTY()
	FGrenadeMove LastMove;
};

UCLASS()
class GWANG_FPS_API AFPSGrenadeBase : public AFPSWeaponBase
{
	GENERATED_BODY()
	
public:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	AFPSGrenadeBase();

	void Server_OnBeginFireWeapon_Implementation() override;
	void Server_OnEndFireWeapon_Implementation() override;

	void Tick(float DeltaSeconds) override;

protected:
	const float GRAVITY = 981.f;

	UPROPERTY(EditDefaultsOnly)
	float ExplodeDelay = 3.f;

	UPROPERTY(EditDefaultsOnly)
	float LaunchSpeed = 3000.f;

	UPROPERTY(EditDefaultsOnly)
	float PathDrawInSeconds = 0.05f;

	UPROPERTY(EditDefaultsOnly)
	float ExplodeRadius = 550.f;

	UPROPERTY(Replicated)
	FGrenadeState ServerState;

	UPROPERTY(Replicated)
	bool bDrawtrajectory;	

	UWorld* World;
	FGrenadeMove LastMove;

protected:
	void BeginPlay() override;

	FGrenadeMove CreateMove();

	UFUNCTION(Server, Unreliable)
	void Server_UpdateMoveState(FGrenadeMove Move, bool bShouldServerSimulateMove);

	void SimulateTrajectory(FGrenadeMove Move);
};

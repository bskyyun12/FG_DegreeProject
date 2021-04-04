// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/FPSWeaponBase.h"
#include "FPSGrenadeBase.generated.h"

/**
 * 
 */
UCLASS()
class GWANG_FPS_API AFPSGrenadeBase : public AFPSWeaponBase
{
	GENERATED_BODY()
	
public:
	void Server_OnBeginFireWeapon_Implementation() override;
	void Server_OnEndFireWeapon_Implementation() override;

	void Client_OnBeginFireWeapon_Implementation() override;
	void Client_OnEndFireWeapon_Implementation() override;

protected:
	UPROPERTY(EditDefaultsOnly)
	float ExplodeDelay = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float LaunchSpeed = 3000.f;

	UPROPERTY(EditDefaultsOnly)
	float PathDrawInSeconds = 0.05f;

	const float GRAVITY = 981.f;
	float LaunchAngleInRad;

	FVector LaunchPoint;
	FVector PrevPoint;
	FVector LaunchForward;
	FVector LaunchUp;

	FTimerHandle GrenadePathTimer;
	float FlightTime = 0.f;
	float LifeTime = 0.f;

protected:
	void BeginPlay() override;

	UFUNCTION()
	void DrawGrenadePath();
};

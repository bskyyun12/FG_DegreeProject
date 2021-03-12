// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/FPSWeaponBase.h"
#include "FPSRifleBase.generated.h"

/**
 * 
 */
UCLASS()
class GWANG_FPS_API AFPSRifleBase : public AFPSWeaponBase
{
	GENERATED_BODY()
	
public:
	void Server_OnBeginFireWeapon_Implementation(AFPSCharacter* FPSCharacter) override;

	UFUNCTION()
	void Fire(AFPSCharacter* FPSCharacter);

	void Server_OnEndFireWeapon_Implementation() override;

private:
	FTransform OwnerCameraTransform;

	FTimerDelegate RifleFireDelegate;
	FTimerHandle RifleFireTimer;

private:
	float CalcDamageToApply(const UPhysicalMaterial* PhysMat);
};

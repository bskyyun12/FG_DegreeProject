// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponBase.h"
#include "GunBase.generated.h"

/**
 * 
 */
UCLASS()
class GWANG_FPS_API AGunBase : public AWeaponBase
{
	GENERATED_BODY()
	
public:
	AGunBase();

	void OnBeginFire() override;
	void Fire() override;
	void OnEndFire() override;

protected:
	FTimerHandle FireTimer;
	FTimerHandle CooldownTimer;
	bool bCooldown;

protected:
	void CalcDamageToApply(const UPhysicalMaterial* PhysMat, float& DamageOnHealth, float& DamageOnArmor);
	bool CanFire() override;

};

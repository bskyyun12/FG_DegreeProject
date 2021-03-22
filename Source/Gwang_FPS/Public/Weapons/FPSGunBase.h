// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/FPSWeaponBase.h"
#include "FPSGunBase.generated.h"

class UCameraShakeBase;

UCLASS()
class GWANG_FPS_API AFPSGunBase : public AFPSWeaponBase
{
	GENERATED_BODY()
	
public:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void Server_Fire_Implementation() override;

	bool CanFire() override;
	
	void Client_Fire_Implementation() override;

	void Client_FireEffects_Implementation() override;

	void Client_OnEndFireWeapon_Implementation() override;

	void Client_Reload_Implementation() override;

	void Server_Reload_Implementation() override;

protected:
	void OnRep_Owner() override;

	float CalcDamageToApply(const UPhysicalMaterial* PhysMat);
	
	void ShakeCamera();
	
	void Recoil();

	void EquipAmmoClip();

	void RemoveAmmoClip();

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCameraShakeBase> CameraShakeOnFire;

	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* RecoilCurve_Vertical;

	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* RecoilCurve_Horizontal;

	FTransform OwnerCameraTransform;
	float RecoilTimer = 0.f;

	UPROPERTY(Replicated, EditDefaultsOnly)
	bool bHasAmmoClip = true;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	int CurrentAmmo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxAmmo = 20;
};

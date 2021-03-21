// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/FPSWeaponBase.h"
#include "FPSRifleBase.generated.h"

class UCameraShakeBase;

UCLASS()
class GWANG_FPS_API AFPSRifleBase : public AFPSWeaponBase
{
	GENERATED_BODY()
	
public:
	void Client_OnFPWeaponEquipped_Implementation(AFPSCharacter* OwnerCharacter) override;
	void Client_OnFPWeaponDroped_Implementation() override;
	void Server_OnTPWeaponEquipped_Implementation(AFPSCharacter* OwnerCharacter) override;
	void Server_OnTPWeaponDroped_Implementation() override;

	void Server_OnBeginFireWeapon_Implementation() override;
	void Server_OnEndFireWeapon_Implementation() override;

	void Client_OnBeginFireWeapon_Implementation() override;
	void Client_OnEndFireWeapon_Implementation() override;

	void Client_Reload_Implementation() override;

	void Server_Fire_Implementation() override;
	void Multicast_FireEffects_Implementation() override;

	void Client_Fire_Implementation() override;
	void Client_FireEffects_Implementation() override;

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCameraShakeBase> CameraShakeOnFire;

	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* RecoilCurve_Vertical;

	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* RecoilCurve_Horizontal;

	FTransform OwnerCameraTransform;
	float RecoilTimer = 0.f;


protected:
	void OnRep_Owner() override;

	float CalcDamageToApply(const UPhysicalMaterial* PhysMat);
	
	void ShakeCamera();
	
	void Recoil();
};

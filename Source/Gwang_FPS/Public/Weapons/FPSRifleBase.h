// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/FPSWeaponBase.h"
#include "FPSRifleBase.generated.h"

class USceneComponent;
class USoundBase;
class UParticleSystem;
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

protected:
	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* FireEmitter;

	UPROPERTY(EditDefaultsOnly)
	USoundBase* FireSound;

	UPROPERTY(EditDefaultsOnly)
	FName FP_WeaponSocketName = "Weapon_Rifle";

	UPROPERTY(EditDefaultsOnly)
	FName TP_WeaponSocketName = "Weapon_Rifle";

	UPROPERTY(EditDefaultsOnly)
	FName FP_MuzzleSocketName = "MuzzleFlash";

	UPROPERTY(EditDefaultsOnly)
	FName TP_MuzzleSocketName = "MuzzleFlash";

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCameraShakeBase> CameraShakeOnFire;	

	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* RecoilCurve_Vertical;

	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* RecoilCurve_Horizontal;

	FTransform OwnerCameraTransform;
	FTimerHandle ServerRifleFireTimer;
	FTimerHandle ClientRifleFireTimer;
	float RecoilTimer = 0.f;

	void OnRep_Owner() override;

protected:
	float CalcDamageToApply(const UPhysicalMaterial* PhysMat);
	
	UFUNCTION()
	void Fire();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_FireEffects();

	void Client_FireEffects();

	void PlayFireEmitter(bool FPWeapon);
	void PlayFireSound(bool FPWeapon);
	void ShakeCamera();
	void Recoil();
};

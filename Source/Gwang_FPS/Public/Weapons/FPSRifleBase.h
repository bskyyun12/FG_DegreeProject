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

	void Client_OnFPWeaponEquipped_Implementation(AFPSCharacter* FPSCharacter) override;
	void Client_OnFPWeaponDroped_Implementation(AFPSCharacter* FPSCharacter) override;
	void Server_OnTPWeaponEquipped_Implementation(AFPSCharacter* FPSCharacter) override;
	void Server_OnTPWeaponDroped_Implementation(AFPSCharacter* FPSCharacter) override;

	void Server_OnBeginFireWeapon_Implementation(AFPSCharacter* FPSCharacter) override;

	UFUNCTION()
	void Fire(AFPSCharacter* FPSCharacter);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_FireEffects();

	void Server_OnEndFireWeapon_Implementation() override;

private:
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

	FTransform OwnerCameraTransform;
	FTimerDelegate RifleFireDelegate;
	FTimerHandle RifleFireTimer;

private:
	float CalcDamageToApply(const UPhysicalMaterial* PhysMat);
};

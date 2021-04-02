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

	// Equip & Drop
	void HandleWeaponEquip() override;

	// Weapon Fire
	bool CanFire() override;
	void CalcDamageToApply(const UPhysicalMaterial* PhysMat, float& DamageOnHealth, float& DamageOnArmor);
	void Server_OnBeginFireWeapon_Implementation() override;
	void Server_Fire_Implementation() override;
	void Server_OnEndFireWeapon_Implementation() override;
	void Client_OnBeginFireWeapon_Implementation() override;
	void Client_OnEndFireWeapon_Implementation() override;

	// Reload
	bool CanReload() override;
	void Client_OnReload_Implementation() override;
	void Server_OnEndReload_Implementation() override;

protected:
	void BeginPlay() override;

	void OnReset() override;

	// Widget Update
	UFUNCTION(Client, Reliable)
	void Client_UpdateAmmoUI(int _CurrentAmmo, int _CurrentRemainingAmmo);

	// Fire Effects
	void Client_FireEffects_Implementation() override;
	void ShakeCamera();
	void Recoil();

protected:
	UPROPERTY(ReplicatedUsing=OnRep_CurrentAmmo)
	int CurrentAmmo = 0;
	UFUNCTION()
	void OnRep_CurrentAmmo();

	UPROPERTY(Replicated)
	int CurrentRemainingAmmo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)	
	int MagazineCapacity = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int RemainingAmmo = 100;

	// Fire Effects
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCameraShakeBase> CameraShakeOnFire;
	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* RecoilCurve_Vertical;
	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* RecoilCurve_Horizontal;
	float RecoilTimer = 0.f;

	FTimerHandle FireTimer_Server;
	FTimerHandle CooldownTimer_Server;

	FTimerHandle FireTimer_Client;
	FTimerHandle CooldownTimer_Client;

	bool bCooldown_Server = false;
	bool bCooldown_Client = false;
};

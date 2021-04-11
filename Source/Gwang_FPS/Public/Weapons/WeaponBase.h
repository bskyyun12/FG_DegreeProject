// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

class ADeathMatchCharacter;
class UAnimMontage;
class USoundBase;

USTRUCT(BlueprintType)
struct FWeaponInfo_Temp
{
	GENERATED_BODY()

	// Stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAutomatic;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ArmorPenetration;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FireRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReloadTime;

	// SocketNames to Equip Weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FP_SocketName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TP_SocketName;

	// Fire Effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystem* FireEmitter;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* FireSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FP_FireEmitterSocketName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TP_FireEmitterSocketName;

	// AnimMontages
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* FP_FireAnimation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* FP_EquipAnim;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* FP_ArmsReloadAnim;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* FP_WeaponReloadAnim;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* TP_ReloadAnim;

	FWeaponInfo_Temp()
	{
		bIsAutomatic = false;
		Damage = 10.f;
		ArmorPenetration = 0.5f;
		Range = 10000.f;
		FireRate = 0.2f;
		ReloadTime = 3.f;

		FP_SocketName = "Weapon_Rifle";
		TP_SocketName = "Weapon_Rifle";

		// Fire Effect
		FireEmitter = nullptr;
		FireSound = nullptr;
		FP_FireEmitterSocketName = "MuzzleFlash";
		TP_FireEmitterSocketName = "MuzzleFlash";

		FP_FireAnimation = nullptr;
		FP_EquipAnim = nullptr;
		FP_ArmsReloadAnim = nullptr;
		FP_WeaponReloadAnim = nullptr;
		TP_ReloadAnim = nullptr;
	}
};

UCLASS()
class GWANG_FPS_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeaponBase();

	FWeaponInfo_Temp GetWeaponInfo() const { return WeaponInfo; };

	UFUNCTION(Client, Reliable)
	void Client_OnWeaponEquipped(ADeathMatchCharacter* NewOwner);

	virtual void OnBeginFire();
	virtual void Fire();
	virtual void OnEndFire();

	// Temp
	FColor GetRoleColor();

protected:

	/** Weapon mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* FPWeaponMesh;

	/** Weapon mesh: 3rd person view (seen only by others) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* TPWeaponMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FWeaponInfo_Temp WeaponInfo;

	UPROPERTY(VisibleAnywhere)
	ADeathMatchCharacter* CurrentOwner;



protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual bool CanFire();

	void OnRep_Owner() override;

};

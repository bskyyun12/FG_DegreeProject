// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponInterface.h"
#include "DeathMatchGameMode.h"
#include "GunBase.generated.h"

class ADeathMatchPlayerController;
class UAnimMontage;
class UBoxComponent;
class USoundBase;

USTRUCT(BlueprintType)
struct FWeaponInfo
{
	GENERATED_BODY()

	// Stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeaponType WeaponType;
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MagazineCapacity = 20;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int RemainingAmmo = 100;

	// Recoil
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCameraShakeBase> CameraShakeOnFire;
	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* RecoilCurve_Vertical;

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

	// Hit Effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystem* HitEmitterOnEnvironment;

	// AnimMontages
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* FP_FireAnimation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* TP_FireAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* FP_EquipAnim;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* FP_ArmsReloadAnim;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* FP_WeaponReloadAnim;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* TP_ReloadAnim;

	FWeaponInfo()
	{
		// Stats
		DisplayName = TEXT("Gweapon");
		WeaponType = EWeaponType::None;
		bIsAutomatic = false;
		Damage = 10.f;
		ArmorPenetration = 0.5f;
		Range = 10000.f;
		FireRate = 0.2f;
		ReloadTime = 3.f;
		MagazineCapacity = 20;
		RemainingAmmo = 60;

		FP_SocketName = "Weapon_Rifle";
		TP_SocketName = "Weapon_Rifle";

		// Recoil
		CameraShakeOnFire = nullptr;
		RecoilCurve_Vertical = nullptr;

		// Fire Effect
		FireEmitter = nullptr;
		FireSound = nullptr;
		FP_FireEmitterSocketName = "MuzzleFlash";
		TP_FireEmitterSocketName = "MuzzleFlash";

		FP_FireAnimation = nullptr;
		TP_FireAnimation = nullptr;
		FP_EquipAnim = nullptr;
		FP_ArmsReloadAnim = nullptr;
		FP_WeaponReloadAnim = nullptr;
		TP_ReloadAnim = nullptr;
	}
};

UCLASS()
class GWANG_FPS_API AGunBase : public AActor, public IWeaponInterface
{
	GENERATED_BODY()
	
public:
	AGunBase();

	// Temp
	FColor GetRoleColor();

	// Getters
	ADeathMatchCharacter* GetCurrentOwner();
	ADeathMatchPlayerController* GetOwnerController();
	EWeaponType GetWeaponType_Implementation() const override;

	// Weapon Equip
	void OnWeaponEquipped_Implementation(ADeathMatchCharacter* NewOwner) override;

	// Weapon Visibility change
	void SetVisibility_Implementation(bool NewVisibility) override;

	// Weapon Drop
	void OnWeaponDropped_Implementation() override;
	
	// Fire
	bool CanFire();
	void BeginFire_Implementation() override;
	void Fire();
	void EndFire_Implementation() override;

	// Reload
	bool CanReload();
	void BeginReload_Implementation() override;
	void OnEndReload();

protected:
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USceneComponent* Root;

	/** Weapon mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* FPWeaponMesh;

	/** Weapon mesh: 3rd person view (seen only by others) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* TPWeaponMesh;
	
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	UBoxComponent* InteractCollider;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FWeaponInfo WeaponInfo;

	UPROPERTY(Replicated)
	int CurrentAmmo = 0;

	UPROPERTY(Replicated)
	int CurrentRemainingAmmo = 0;

	// Recoil
	float RecoilTimer = 0.f;

	// Fire
	FTimerHandle FireTimer;
	FTimerHandle CooldownTimer;
	bool bFireCooldown;

	// Reload
	FTimerHandle ReloadTimer;
	bool bIsReloading;

protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void BeginPlay() override;

	// Weapon fire
	bool FireLineTrace(FHitResult& OutHit);
	void CalcDamageToApply(const UPhysicalMaterial* PhysMat, float& DamageOnHealth, float& DamageOnArmor);
	void FireEffects();
	void Recoil();

	// UI
	void UpdateAmmoUI(const int& InCurrentAmmo, const int& InRemainingAmmo);

};

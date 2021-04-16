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
struct FGunInfo
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

	FGunInfo()
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

		// Hit Effects
		HitEmitterOnEnvironment = nullptr;

		// AnimMontages
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

	#pragma region Getters & Setters
	ADeathMatchCharacter* GetCurrentOwner();
	ADeathMatchPlayerController* GetOwnerController();
	EWeaponType GetWeaponType_Implementation() const override;

	void SetVisibility_Implementation(bool NewVisibility) override;
	#pragma endregion Getters & Setters

	// Weapon Equip & Drop
	void OnWeaponEquipped_Implementation(ADeathMatchCharacter* NewOwner) override;
	void OnWeaponDropped_Implementation() override;
	
	// Fire
	void BeginFire_Implementation() override;
	void EndFire_Implementation() override;

	// Reload
	void BeginReload_Implementation() override;

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
	FGunInfo WeaponInfo;

	// Ammo
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
	bool CanFire();
	void Fire();
	bool FireLineTrace(FHitResult& OutHit);
	void CalcDamageToApply(const UPhysicalMaterial* PhysMat, float& DamageOnHealth, float& DamageOnArmor);

	// Fire Effects
	void FireEffects();
	UFUNCTION(Server, Unreliable)
	void Server_FireEffects();
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_FireEffects();

	// Impact Effect
	void ImpactEffect(const FVector& ImpactPoint);
	UFUNCTION(Server, Unreliable)
	void Server_ImpactEffect(const FVector& ImpactPoint);
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_ImpactEffect(const FVector& ImpactPoint);

	// Reload
	bool CanReload();

	void Reload();
	UFUNCTION(Server, Reliable)
	void Server_Reload();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Reload();
	void OnEndReload();

	// Recoil
	void Recoil();

	// UI
	void UpdateAmmoUI(const int& InCurrentAmmo, const int& InRemainingAmmo);
};

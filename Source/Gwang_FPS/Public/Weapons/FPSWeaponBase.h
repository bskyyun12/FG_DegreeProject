// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSWeaponInterface.h"
#include "FPSWeaponBase.generated.h"

class UAnimMontage;
class USceneComponent;
class USkeletalMeshComponent;
class AFPSCharacter;
class USphereComponent;

class UParticleSystem;
class USoundBase;

USTRUCT(BlueprintType)
struct FWeaponInfo
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
	FName FP_ArmsSocketName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TP_CharacterSocketName;

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
	UAnimMontage* FP_EquipAnim;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* FP_ArmsReloadAnim;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* FP_WeaponReloadAnim;

	FWeaponInfo() 
	{
		// Stats
		bIsAutomatic = false;
		Damage = 10.f;
		ArmorPenetration = 0.5f;
		Range = 10000.f;
		FireRate = 0.2f;
		ReloadTime = 3.f;

		// Equip Weapon
		FP_ArmsSocketName = "Weapon_Rifle";
		TP_CharacterSocketName = "Weapon_Rifle";

		// Fire Effect
		FireEmitter = nullptr;
		FireSound = nullptr;
		FP_FireEmitterSocketName = "MuzzleFlash";
		TP_FireEmitterSocketName = "MuzzleFlash";

		// AnimMontages
		FP_EquipAnim = nullptr;
		FP_ArmsReloadAnim = nullptr;
		FP_WeaponReloadAnim = nullptr;
	}
};

UCLASS()
class GWANG_FPS_API AFPSWeaponBase : public AActor, public IFPSWeaponInterface
{
	GENERATED_BODY()
	
public:	
	AFPSWeaponBase();

	// Equip & Drop
	UFUNCTION(Server, Reliable)
	void Server_OnWeaponEquipped(AFPSCharacter* OwnerCharacter);
	UFUNCTION(Server, Reliable)
	void Server_OnWeaponDroped();

	// Weapon Fire
	UFUNCTION(Server, Reliable)
	void Server_OnBeginFireWeapon();
	UFUNCTION(Server, Reliable)
	void Server_OnEndFireWeapon();
	UFUNCTION(Client, Reliable)
	void Client_OnBeginFireWeapon();
	UFUNCTION(Client, Reliable)
	void Client_OnEndFireWeapon();

	// Reload
	virtual bool CanReload();
	UFUNCTION(Server, Reliable)
	void Server_OnReload();
	UFUNCTION(Client, Reliable)
	void Client_OnReload();
	UFUNCTION(Server, Reliable)
	void Server_OnEndReload();

	// Getters
	AFPSWeaponBase* GetWeapon_Implementation() override; // IFPSWeaponInterface
	FWeaponInfo GetWeaponInfo();

protected:
	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Called when Owner changed
	void OnRep_Owner() override;

	// Fire
	UFUNCTION()
	virtual bool CanFire();
	UFUNCTION(Server, Reliable)
	void Server_Fire();
	UFUNCTION(Client, Reliable)
	void Client_Fire();

	// Fire Effects
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_FireEffects();
	UFUNCTION(Client, Unreliable)
	void Client_FireEffects();
	void PlayFireEmitter(bool FPWeapon);
	void PlayFireSound(bool FPWeapon);

protected:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* RootComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* FPWeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* TPWeaponMesh;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* InteractCollider;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FWeaponInfo WeaponInfo;

	// Reload
	UPROPERTY(Replicated)
	bool bIsReloading = false;

	FTimerHandle ReloadTimer;
};

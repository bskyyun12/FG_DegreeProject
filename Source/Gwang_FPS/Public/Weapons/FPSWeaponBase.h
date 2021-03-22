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

	/////////
	// Stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAutomatic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FireRate;
	// Stats
	/////////

	///////////////
	// Equip Weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FP_ArmsSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TP_CharacterSocketName;
	// Equip Weapon
	///////////////

	//////////////
	// Fire Effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystem* FireEmitter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FP_FireEmitterSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TP_FireEmitterSocketName;
	// Fire Effects
	//////////////

	///////////////
	// AnimMontages
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* FP_EquipAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* FP_ArmsReloadAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* FP_WeaponReloadAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* TP_ReloadAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* HideAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* FireAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* IdleAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* WalkAnim;
	// AnimMontages
	///////////////

	FWeaponInfo() 
	{
		// Stats
		bIsAutomatic = false;
		Damage = 10.f;
		Range = 10000.f;
		FireRate = 0.2f;

		// Equip Weapon
		FP_ArmsSocketName = "Weapon_Rifle";
		TP_CharacterSocketName = "Weapon_Rifle";

		// Fire Effect
		FireEmitter = nullptr;
		FireSound = nullptr;
		FP_FireEmitterSocketName = "MuzzleFlash";
		TP_FireEmitterSocketName = "MuzzleFlash";

		// Anims
		FP_EquipAnim = nullptr;
		FP_ArmsReloadAnim = nullptr;
		FP_WeaponReloadAnim = nullptr;
		TP_ReloadAnim = nullptr;
		HideAnim = nullptr;
		FireAnim = nullptr;
		IdleAnim = nullptr;
		WalkAnim = nullptr;
	}
};

UCLASS()
class GWANG_FPS_API AFPSWeaponBase : public AActor, public IFPSWeaponInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSWeaponBase();

	AFPSWeaponBase* GetWeapon_Implementation() override;

	UFUNCTION(Server, Reliable)
	virtual void Server_OnBeginFireWeapon();

	UFUNCTION(Server, Reliable)
	virtual void Server_OnEndFireWeapon();

	UFUNCTION(Client, Reliable)
	virtual void Client_OnBeginFireWeapon();

	UFUNCTION(Client, Reliable)
	virtual void Client_OnEndFireWeapon();

	/////////////////////////
	// FP Weapon (Local Only)
	UFUNCTION(Client, Reliable)
	virtual void Client_OnFPWeaponEquipped(AFPSCharacter* OwnerCharacter);

	UFUNCTION(Client, Reliable)
	virtual void Client_OnFPWeaponDroped();

	UFUNCTION(Client, Reliable)
	virtual void Client_Reload();
		
	UFUNCTION(Server, Reliable)
	virtual void Server_Reload();
	// FP Weapon (Local Only)
	/////////////////////////

	/////////////////////////
	// TP Weapon (Should be replicated)
	UFUNCTION(Server, Reliable)
	virtual void Server_OnTPWeaponEquipped(AFPSCharacter* OwnerCharacter);

	UFUNCTION(Server, Reliable)
	virtual void Server_OnTPWeaponDroped();
	// TP Weapon (Should be replicated)
	/////////////////////////

protected:
	virtual void BeginPlay() override;

	void OnRep_Owner() override;

	///////
	// Fire
	UFUNCTION()
	virtual bool CanFire();

	UFUNCTION(Server, Reliable)
	virtual void Server_Fire();

	UFUNCTION(NetMulticast, Unreliable)
	virtual void Multicast_FireEffects();

	UFUNCTION(Client, Reliable)
	virtual void Client_Fire();

	UFUNCTION(Client, Unreliable)
	virtual void Client_FireEffects();

	void PlayFireEmitter(bool FPWeapon);
	void PlayFireSound(bool FPWeapon);
	// Fire
	///////

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

	FTimerHandle ServerAutomaticFireTimer;
	FTimerHandle ClientAutomaticFireTimer;
};

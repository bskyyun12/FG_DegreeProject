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

// Each enum value represents index of weapon array in FPSCharacter
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	None,
	MainWeapon,
	SubWeapon,
	Melee,
	Grenade,

	EnumSize,	// Leave the enum value as default
};

UENUM(BlueprintType)
enum class EMainWeapon : uint8	// NAME CHANGE? Make sure to change the name in ULobbyInventory::Initialize!
{
	M4A1,
	AK47,
	EnumSize
};

UENUM(BlueprintType)
enum class ESubWeapon : uint8	// NAME CHANGE? Make sure to change the name in ULobbyInventory::Initialize!
{
	Pistol,
	EnumSize
};

UENUM(BlueprintType)
enum class EKnife : uint8	// NAME CHANGE? Make sure to change the name in ULobbyInventory::Initialize!
{
	Knife,
	EnumSize
};

UENUM(BlueprintType)
enum class EGrenade : uint8	// NAME CHANGE? Make sure to change the name in ULobbyInventory::Initialize!
{
	Grenade,
	EnumSize
};

USTRUCT(BlueprintType)
struct FWeaponClass
{
	GENERATED_BODY()

	// Main Weapons
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AFPSWeaponBase> M4A1Class;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AFPSWeaponBase> PistolClass;

	// Sub Weapons
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AFPSWeaponBase> AK47Class;

	// Knives
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AFPSWeaponBase> KnifeClass;

	// Grenades
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AFPSWeaponBase> GrenadeClass;

};

USTRUCT(BlueprintType)
struct FWeaponInfo
{
	GENERATED_BODY()

	// Stats
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

	// Hit Effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystem* HitEmitter;

	// AnimMontages
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
		WeaponType = EWeaponType::None;
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

		// Hit Effects
		HitEmitter = nullptr;

		// AnimMontages
		FP_EquipAnim = nullptr;
		FP_ArmsReloadAnim = nullptr;
		FP_WeaponReloadAnim = nullptr;
		TP_ReloadAnim = nullptr;
	}
};

UCLASS()
class GWANG_FPS_API AFPSWeaponBase : public AActor, public IFPSWeaponInterface
{
	GENERATED_BODY()
	
public:
	AFPSWeaponBase();

	virtual void OnReset();

	// Equip & Hide & Drop
	void ToggleVisibility(bool bNewVisibility);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ToggleVisibility(bool bNewVisibility);
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

	// Weapon Equip & Drop
	virtual void HandleWeaponEquip();
	virtual void HandleWeaponDrop();

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USphereComponent* InteractCollider;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FWeaponInfo WeaponInfo;

	// Reload
	UPROPERTY(Replicated)
	bool bIsReloading = false;

	FTimerHandle ReloadTimer;
};

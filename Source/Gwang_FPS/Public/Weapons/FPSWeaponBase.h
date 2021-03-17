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

USTRUCT(BlueprintType)
struct FWeaponInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FireRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* FP_EquipAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* FP_ArmsReploadAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* FP_WeaponReploadAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* HideAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* FireAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* IdleAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* WalkAnim;

	FWeaponInfo() {}
};

UENUM(BlueprintType)
enum class EWeaponType : uint8 {
	Rifle	UMETA(DisplayName = "Rifle"),
	Pistol	UMETA(DisplayName = "Pistol"),
};

UCLASS()
class GWANG_FPS_API AFPSWeaponBase : public AActor, public IFPSWeaponInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSWeaponBase();

	AFPSWeaponBase* GetWeapon_Implementation() override;

	EWeaponType GetWeaponType();
	
	/////////////////////////
	// FP Weapon (Local Only)
	UFUNCTION(Client, Reliable)
	void Client_OnFPWeaponEquipped(AFPSCharacter* OwnerCharacter);

	UFUNCTION(Client, Reliable)
	void Client_OnFPWeaponDroped();

	UFUNCTION(Client, Reliable)
	void Client_Reload();
	// FP Weapon (Local Only)
	/////////////////////////

	/////////////////////////
	// TP Weapon (Should be replicated)
	UFUNCTION(Server, Reliable)
	void Server_OnTPWeaponEquipped(AFPSCharacter* OwnerCharacter);

	UFUNCTION(Server, Reliable)
	void Server_OnTPWeaponDroped();
	// TP Weapon (Should be replicated)
	/////////////////////////

	UFUNCTION(Server, Reliable)
	void Server_OnBeginFireWeapon();

	UFUNCTION(Server, Reliable)
	void Server_OnEndFireWeapon();

	UFUNCTION(Client, Reliable)
	void Client_OnBeginFireWeapon();

	UFUNCTION(Client, Reliable)
	void Client_OnEndFireWeapon();

protected:
	UFUNCTION()
	bool CanFire();

	void OnRep_Owner() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* RootComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* FPWeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* TPWeaponMesh;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* InteractCollider;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FWeaponInfo WeaponInfo;
};

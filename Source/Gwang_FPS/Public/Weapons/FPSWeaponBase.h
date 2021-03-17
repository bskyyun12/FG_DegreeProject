// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSWeaponInterface.h"
#include "FPSWeaponBase.generated.h"

class UAnimMontage;
class UAnimSequence;
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
	UAnimSequence* EquipAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* HideAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* FireAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* ReploadAnim;

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
	void Client_OnFPWeaponEquipped(AFPSCharacter* FPSCharacter);

	UFUNCTION(Client, Reliable)
	void Client_OnFPWeaponDroped(AFPSCharacter* FPSCharacter);
	// FP Weapon (Local Only)
	/////////////////////////

	/////////////////////////
	// TP Weapon (Should be replicated)
	UFUNCTION(Server, Reliable)
	void Server_OnTPWeaponEquipped(AFPSCharacter* FPSCharacter);

	UFUNCTION(Server, Reliable)
	void Server_OnTPWeaponDroped(AFPSCharacter* FPSCharacter);
	// TP Weapon (Should be replicated)
	/////////////////////////

	UFUNCTION(Server, Reliable)
	void Server_OnBeginFireWeapon(AFPSCharacter* FPSCharacter);

	UFUNCTION(Server, Reliable)
	void Server_OnEndFireWeapon();

	UFUNCTION(Client, Reliable)
	void Client_OnBeginFireWeapon();

	UFUNCTION(Client, Reliable)
	void Client_OnEndFireWeapon();

protected:
	UPROPERTY(ReplicatedUsing = OnRep_OwnerChanged)
	AFPSCharacter* OwnerCharacter;

	UFUNCTION()
	void OnRep_OwnerChanged();

	UFUNCTION()
	bool CanFire();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* RootComp;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* FPWeaponMesh;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* TPWeaponMesh;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* InteractCollider;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FWeaponInfo WeaponInfo;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSWeaponBase.generated.h"

class UAnimMontage;
class USceneComponent;
class USkeletalMeshComponent;
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
	UAnimMontage* EquipAnim;

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
class GWANG_FPS_API AFPSWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSWeaponBase();

	EWeaponType GetWeaponType();

#pragma region FP Weapon (Local Only)
	UFUNCTION(Client, Reliable)
	void Client_OnClientWeaponEquipped(USkeletalMeshComponent* FPSArmMesh);

	UFUNCTION(Client, Reliable)
	void Client_OnClientWeaponDroped(USkeletalMeshComponent* FPSArmMesh);
#pragma endregion

#pragma region TP Weapon (Should be replicated)
	UFUNCTION(Server, Reliable)
	void Server_OnRepWeaponEquipped(USkeletalMeshComponent* FPSCharacterMesh);

	UFUNCTION(Server, Reliable)
	void Server_OnRepWeaponDroped(USkeletalMeshComponent* FPSCharacterMesh);
#pragma endregion

	UPROPERTY(ReplicatedUsing = OnRep_OwnerChanged)
	USkeletalMeshComponent* OwnerCharacterMesh;

	UFUNCTION()
	void OnRep_OwnerChanged();

	UFUNCTION(Server, Reliable)
	void Server_FireWeapon(FTransform CameraTransform);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(VisibleAnywhere)
	USceneComponent* RootComp;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* ClientWeaponMesh;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* RepWeaponMesh;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* InteractCollider;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FWeaponInfo WeaponInfo;

};

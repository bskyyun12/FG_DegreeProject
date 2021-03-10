// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPSCharacterInterface.h"
#include "FPSCharacter.generated.h"

class AFPSPlayerController;
class UBoxComponent;
class UCameraComponent;
class USkeletalMeshComponent;
class UHealthComponent;

UCLASS()
class GWANG_FPS_API AFPSCharacter : public ACharacter, public IFPSCharacterInterface
{
	GENERATED_BODY()

public:
	AFPSCharacter();

	void OnPossessed(AFPSPlayerController* InFPSController);

#pragma region Input binds
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Pickup();
	void Fire();
	UFUNCTION(Server, Reliable)
	void Server_Fire(AFPSWeaponBase* Weapon, FTransform CameraTransform);
#pragma endregion

	bool HasWeapon(EWeaponType WeaponType);
	void PickupWeapon(EWeaponType WeaponType);

	void EquipWeapon(AFPSWeaponBase* Weapon);
	UFUNCTION(Server, Reliable)
	void Server_EquipWeapon(AFPSWeaponBase* Weapon);

#pragma region IFPSCharacterInterface
	void OnBeginOverlapWeapon_Implementation(AFPSWeaponBase* Weapon) override;
	void OnEndOverlapWeapon_Implementation() override;
#pragma endregion

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* CameraContainer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* FPSArms;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UHealthComponent* HealthComponent;

	TMap<EWeaponType, int16> Weapons;
	AFPSWeaponBase* CurrentWeapon;

	AFPSWeaponBase* CurrentFocus;
	FTimerHandle PickupTraceTimerHandle;
	int NumOfOverlappingWeapons;

	float RespawnDelay = 5.f;

	// Cache
	USkeletalMeshComponent* CharacterMesh;
	UCapsuleComponent* CapsuleComponent;
	AFPSPlayerController* FPSController;
	FVector DefaultCameraRelativeLocation;

	// Temporary thing
	bool bHasAnyWeapons;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(Client, Unreliable)
	void Client_CheckForWeapon();

#pragma region Health & Death
	UFUNCTION()
	void OnDamageReceived();

	UFUNCTION()
	void OnHealthAcquired();
		
	UFUNCTION()
	void OnDeath();

	UPROPERTY(ReplicatedUsing = OnRep_bIsDead)
	bool bIsDead;
	UFUNCTION()
	void OnRep_bIsDead();

	void RespawnPlayer();

	void CollisionHandleOnDeath();
	void CollisionHandleOnRespawn();

#pragma endregion Health & Death
};

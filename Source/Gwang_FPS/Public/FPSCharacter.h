// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPSCharacterInterface.h"
#include "Weapons/FPSWeaponBase.h"
#include "FPSCharacter.generated.h"

class AFPSPlayerController;
class UBoxComponent;
class UCameraComponent;
class USkeletalMeshComponent;
class UHealthComponent;
class UFPSGameInstance;

UCLASS()
class GWANG_FPS_API AFPSCharacter : public ACharacter, public IFPSCharacterInterface
{
	GENERATED_BODY()

	DECLARE_DELEGATE_OneParam(FOneBooleanDelegate, bool)

public:
	AFPSCharacter();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma region Input bindings
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	UFUNCTION(Server, Unreliable)
	void Server_LookUp(FRotator CameraRot);
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_LookUp(FRotator CameraRot);

	void OnBeginFire();
	UFUNCTION(Server, Reliable)
	void Server_OnBeginFire(AFPSWeaponBase* Weapon);
	void OnEndFire();
	UFUNCTION(Server, Reliable)
	void Server_OnEndFire(AFPSWeaponBase* Weapon);

	// Weapon Equip & Swap
	void EquipMainWeapon();
	UFUNCTION(Server, Reliable)
	void Server_EquipMainWeapon();
	void EquipSubWeapon();
	UFUNCTION(Server, Reliable)
	void Server_EquipSubWeapon();
	UFUNCTION(Server, Reliable)
	void Server_EquipWeapon(AFPSWeaponBase* Weapon);

	void Drop();
	UFUNCTION(Server, Reliable)
	void Server_DropWeapon(AFPSWeaponBase* Weapon);

	void Reload();
	UFUNCTION(Server, Reliable)
	void Server_Reload(AFPSWeaponBase* Weapon);
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayReloadAnim(AFPSWeaponBase* Weapon);

	void ToggleScoreBoardWidget(bool bDisplay);

	void HandleCrouch(bool bCrouchButtonDown);
	UFUNCTION(Server, Unreliable)
	void Server_HandleCrouch(bool bCrouchButtonDown);
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_HandleCrouch(bool bCrouchButtonDown);
#pragma endregion Input bindings

#pragma region IFPSCharacterInterface
	// Getters
	FTransform GetCameraTransform_Implementation() override;
	USkeletalMeshComponent* GetCharacterMesh_Implementation() override;
	USkeletalMeshComponent* GetArmMesh_Implementation() override;
	float GetHealth_Implementation() override;
	float GetArmor_Implementation() override;

	void OnSpawnPlayer_Implementation() override;
	void TakeDamage_Implementation(AActor* DamageCauser, float DamageOnHealth, float DamageOnArmor) override;
#pragma endregion IFPSCharacterInterface


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* FPSArmMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UHealthComponent* HealthComponent;

	UPROPERTY(Replicated=OnRep_CurrentWeapon, VisibleAnywhere, BlueprintReadOnly)
	AFPSWeaponBase* CurrentWeapon;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	AFPSWeaponBase* MainWeapon;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	AFPSWeaponBase* SubWeapon;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AFPSWeaponBase> RifleClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AFPSWeaponBase> PistolClass;

	// Cache
	UPROPERTY()
	UFPSGameInstance* FPSGameInstance;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


	// Crouch
	FTimerHandle CrouchTimerHandle;
	FVector CameraRelativeLocation_Default;
	UPROPERTY(EditDefaultsOnly)
	FVector CameraRelativeLocationOnCrouch = FVector::ZeroVector;
	FVector DesiredCameraRelativeLocation;
	UFUNCTION()
	void CrouchTimer();

#pragma region Health & Spawn & Death
	UFUNCTION(BlueprintPure)
	bool IsDead();

	UFUNCTION()
	void OnTakeDamage(AActor* DamageSource);

	UFUNCTION()
	void OnHealthAcquired(AActor* HealthSource);

	UFUNCTION()
	void OnUpdateHealthArmorUI();

	UFUNCTION()
	void OnDeath(AActor* DeathSource);
	void HandleCollisionOnDeath();
	UFUNCTION(Server, Reliable)
	void Server_WeaponSetupOnSpawn();
	
	UFUNCTION()
	void OnSpawn();
	void HandleCollisionOnSpawn();
#pragma endregion Spawn & Death
};

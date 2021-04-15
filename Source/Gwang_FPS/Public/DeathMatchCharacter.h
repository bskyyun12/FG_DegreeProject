// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPSGameInstance.h"
#include "DeathMatchGameMode.h"
#include "DeathMatchCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class AGunBase;
class ADeathMatchPlayerState;
class UFPSGameInstance;

UCLASS()
class GWANG_FPS_API ADeathMatchCharacter : public ACharacter
{
	GENERATED_BODY()


public:	
	ADeathMatchCharacter();

	// Getters
	ADeathMatchPlayerState* GetPlayerState();
	AActor* GetCurrentWeapon();
	AActor* GetCurrentMainWeapon();
	AActor* GetCurrentSubWeapon();
	bool IsDead();
	USkeletalMeshComponent* GetArmMesh() const { return ArmMesh; }
	FVector GetCameraLocation() const;
	FVector GetCameraForward() const;

	// Setters
	void SetCurrentlyHeldWeapon(AActor* NewWeapon);
	void SetCameraWorldRotation(const FRotator& Rotation);

	UFUNCTION(Server, Reliable)
	void Server_OnSpawnPlayer();

	// TakeDamage
	UFUNCTION(Server, Reliable)
	void Server_TakeDamage(const uint8& DamageOnHealth, const uint8& DamageOnArmor, AActor* DamageCauser);

protected:
	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* ArmMesh;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FP_Camera;

	/** Camera to activate on death */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCameraComponent* DeathCamera;

	// Cache
	UFPSGameInstance* GI;
	ADeathMatchPlayerState* PS;

protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay();

	DECLARE_DELEGATE_OneParam(FOneBooleanDelegate, bool)
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	void MoveForward(float Val);
	void MoveRight(float Val);

	// Weapon Equip & Swap
	void EquipMainWeapon();
	void EquipSubWeapon();
	UFUNCTION(Server, Reliable)
	void Server_EquipWeapon(AActor* WeaponToEquip);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EquipWeapon(AActor* WeaponToEquip);
	void EquipWeapon(AActor* WeaponToEquip);

	// Weapon drop
	void Drop();
	UFUNCTION(Server, Reliable)
	void Server_DropWeapon();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DropWeapon();
	void DropWeapon();

	// Weapon pickup
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	void PickupWeapon(AActor* WeaponToPickup);
	UFUNCTION(Server, Reliable)
	void Server_PickupWeapon(AActor* Weapon);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PickupWeapon(AActor* Weapon);

	// Weapon Reload
	void BeginReload();
	UFUNCTION(Server, Reliable)
	void Server_BeginReload();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_BeginReload();
	void Reload();

	// Toggle ScoreBoard
	void ToggleScoreBoardWidget(bool bDisplay);

	// Chat
	void StartChat();

	// Look up
	void LookUp(float Value);
	UFUNCTION(Server, Reliable)
	void Server_LookUp(const FRotator& CameraRotation);
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_LookUp(const FRotator& CameraRotation);

	// Begin Fire
	void OnBeginFire();
	UFUNCTION(Server, Reliable)
	void Server_OnBeginFire();
	UPROPERTY(ReplicatedUsing=OnRep_BeginFire)
	uint8 BeginFire;
	UFUNCTION()
	void OnRep_BeginFire();

	// End Fire
	void OnEndFire();
	UFUNCTION(Server, Reliable)
	void Server_OnEndFire();
	UPROPERTY(ReplicatedUsing = OnRep_EndFire)
	uint8 EndFire;
	UFUNCTION()
	void OnRep_EndFire();

	// OnSpawn
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnSpawn();
	void HandleCameraOnSpawn();

	// TakeDamage
	UFUNCTION(Client, Reliable)
	void Client_OnTakeDamage();

	// OnDeath
	UFUNCTION(Server, Reliable)
	void Server_OnDeath(AActor* DeathCauser);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnDeath();
	void HandleCameraOnDeath();

	// OnKill
	UFUNCTION(Server, Reliable)
	void Server_OnKill(ADeathMatchCharacter* DeadPlayer);

	// Crouch
	void HandleCrouch(bool bCrouchButtonDown);
	FTimerHandle CrouchTimerHandle;
	FVector CameraRelativeLocation_Default;
	UPROPERTY(EditDefaultsOnly)
	FVector CameraRelativeLocationOnCrouch = FVector::ZeroVector;
	FVector DesiredCameraRelativeLocation;
	UFUNCTION()
	void CrouchSimulate();
	UFUNCTION(Server, Reliable)
	void Server_HandleCrouch(bool bCrouchButtonDown);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HandleCrouch(bool bCrouchButtonDown);
};

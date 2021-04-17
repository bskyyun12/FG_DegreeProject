// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DeathMatchCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class ADeathMatchPlayerState;
class UFPSGameInstance;

UCLASS()
class GWANG_FPS_API ADeathMatchCharacter : public ACharacter
{
	GENERATED_BODY()

public:	
	ADeathMatchCharacter();

	#pragma region Getter & Setters
	// Getters
	ADeathMatchPlayerState* GetPlayerState();
	AActor* GetCurrentlyHeldWeapon();
	AActor* GetCurrentMainWeapon();
	AActor* GetCurrentSubWeapon();
	AActor* GetCurrentMeleeWeapon();
	AActor* GetCurrentGrenade();
	bool IsDead();
	USkeletalMeshComponent* GetArmMesh() const { return ArmMesh; }
	FVector GetCameraLocation() const;
	FVector GetCameraForward() const;
	float GetCameraPitch() const;

	// Setters
	void SetCurrentlyHeldWeapon(AActor* NewWeapon);
	void SetCameraWorldRotation(const FRotator& Rotation);
	void SetCurrentWeaponWithIndex(const uint8& Index, AActor* NewWeapon);
	#pragma endregion Getter & Setters

	// Called after ADeathMatchPlayerController::Server_OnSpawnPlayer
	UFUNCTION(Server, Reliable)
	void Server_OnSpawnPlayer();

	// TakeDamage
	UFUNCTION(Server, Reliable)
	void Server_TakeDamage(const uint8& DamageOnHealth, const uint8& DamageOnArmor, AActor* DamageCauser);
	UFUNCTION(Client, Reliable)
	void Client_OnTakeDamage();

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
	ADeathMatchPlayerState* PS;

protected:
	virtual void BeginPlay();

	DECLARE_DELEGATE_OneParam(FOneBooleanDelegate, bool)
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	#pragma region Move & Turn
	void MoveForward(float Val);
	void MoveRight(float Val);

	void LookUp(float Value);
	UFUNCTION(Server, Reliable)
	void Server_LookUp(const FRotator& CameraRotation);
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_LookUp(const FRotator& CameraRotation);
	#pragma endregion Move & Turn
	
	#pragma region Spawn & Death
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnSpawn();
	void HandleCameraOnSpawn();

	UFUNCTION(Server, Reliable)
	void Server_OnDeath(AActor* DeathCauser);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnDeath();
	void HandleCameraOnDeath();

	UFUNCTION(Server, Reliable)
	void Server_OnKill(ADeathMatchCharacter* DeadPlayer);
	#pragma endregion Spawn & Death

	#pragma region Weapon Equip & Swap
	void EquipMainWeapon();
	void EquipSubWeapon();
	void EquipMeleeWeapon();
	void EquipGrenade();
	UFUNCTION(Server, Reliable)
	void Server_EquipWeapon(AActor* WeaponToEquip);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EquipWeapon(AActor* WeaponToEquip);
	void EquipWeapon(AActor* WeaponToEquip);
	#pragma endregion Weapon Equip & Swap

	#pragma region Weapon Pickup & Drop
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	void PickupWeapon(AActor* WeaponToPickup);
	UFUNCTION(Server, Reliable)
	void Server_PickupWeapon(AActor* Weapon);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PickupWeapon(AActor* Weapon);

	void Drop();
	UFUNCTION(Server, Reliable)
	void Server_DropWeapon();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DropWeapon();
	void DropWeapon();
	#pragma endregion Weapon Pickup & Drop

	#pragma region Weapon Fire
	void OnBeginFire();
	UFUNCTION(Server, Reliable)
	void Server_OnBeginFire();

	void OnEndFire();
	UFUNCTION(Server, Reliable)
	void Server_OnEndFire();
	#pragma endregion Weapon Fire

	#pragma region Weapon Reload
	void BeginReload();
	UFUNCTION(Server, Reliable)
	void Server_BeginReload();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_BeginReload();
	void Reload();
	#pragma endregion Weapon Reload

	#pragma region Crouch
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
	#pragma endregion Crouch

	// Toggle ScoreBoard
	void ToggleScoreBoardWidget(bool bDisplay);

	// Chat
	void StartChat();
};

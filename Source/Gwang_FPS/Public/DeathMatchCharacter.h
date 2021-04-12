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
class ADeathMatchGameMode;

UCLASS()
class GWANG_FPS_API ADeathMatchCharacter : public ACharacter
{
	GENERATED_BODY()

	DECLARE_DELEGATE_OneParam(FOneBooleanDelegate, bool)

public:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	ADeathMatchCharacter();

	USkeletalMeshComponent* GetArmMesh() const { return ArmMesh; }
	FVector GetCameraLocation() const;

	bool IsServerPlayer() const { return GetLocalRole() == ROLE_Authority && IsLocallyControlled(); }

	// TakeDamage
	UFUNCTION(Server, Reliable)
	void Server_TakeDamage(float DamageOnHealth, float DamageOnArmor, AActor* DamageCauser);

protected:
	//UPROPERTY(EditDefaultsOnly, Category = Weapon)
	//TSubclassOf<AGunBase> RifleClass;

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* ArmMesh;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FP_Camera;

	/** Camera to activate on death */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCameraComponent* DeathCamera;

	// Weapons
	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> StartWeapons;

	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> CurrentWeapons;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentlyHeldWeapon, BlueprintReadOnly)
	AActor* CurrentlyHeldWeapon;
	UFUNCTION()
	void OnRep_CurrentlyHeldWeapon();

	// Health & Armor
	float CurrentHealth = 100.f;
	float CurrentArmor = 100.f;

	// Cache
	ADeathMatchGameMode* GM;
	UFPSGameInstance* GI;
	ADeathMatchPlayerState* PS;

protected:
	virtual void BeginPlay();

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	void MoveForward(float Val);
	void MoveRight(float Val);

	// Possession
	void PossessedBy(AController* NewController) override;
	void UnPossessed() override;

	// Weapon Equip & Swap
	void EquipMainWeapon();
	void EquipSubWeapon();
	UFUNCTION(Server, Reliable)
	void Server_EquipWeapon(AActor* WeaponToEquip);
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
	void PickupWeapon(AActor* const& WeaponToPickup);

	// Weapon Reload
	void Reload();
	UFUNCTION(Server, Reliable)
	void Server_Reload();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Reload();

	// Toggle ScoreBoard
	void ToggleScoreBoardWidget(bool bDisplay);

	// Chat
	void StartChat();

	// Look up
	void LookUp(float Value);
	UFUNCTION(Server, Reliable)
	void Server_LookUp(const float& Value);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_LookUp(const float& Value);

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

	// UI
	UFUNCTION(Client, Reliable)
	void Client_UpdateHealthArmorUI(const uint8& Health, const uint8& Armor);

	// OnSpawn
	UFUNCTION(Server, Reliable)
	void Server_OnSpawn();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnSpawn(const FWeaponClass& WeaponClass);
	void HandleCameraOnSpawn();

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

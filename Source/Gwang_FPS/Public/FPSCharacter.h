// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPSCharacterInterface.h"
#include "Weapons/FPSWeaponBase.h"
#include "FPSGameInstance.h"
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
	void SwitchToMainWeapon();
	void SwitchToSubWeapon();

	void Drop();
	UFUNCTION(Server, Reliable)
	void Server_DropWeapon();

	void Reload();
	UFUNCTION(Server, Reliable)
	void Server_Reload(AFPSWeaponBase* Weapon);
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayReloadAnim(AFPSWeaponBase* Weapon);

	void ToggleScoreBoardWidget(bool bDisplay);

	void HandleCrouch(bool bCrouchButtonDown);
	UFUNCTION(Server, Reliable)
	void Server_HandleCrouch(bool bCrouchButtonDown);
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_HandleCrouch(bool bCrouchButtonDown);

	void OnEscapeButtonPressed();

	void StartChat();
#pragma endregion Input bindings

#pragma region IFPSCharacterInterface
	// Getters
	FTransform GetCameraTransform_Implementation() override;
	USkeletalMeshComponent* GetCharacterMesh_Implementation() override;
	USkeletalMeshComponent* GetArmMesh_Implementation() override;
	float GetHealth_Implementation() override;
	float GetArmor_Implementation() override;

	void OnSpawnPlayer_Implementation() override;
	void TakeDamage_Implementation(AActor* DamageCauser, float DamageOnHealth, float DamageOnArmor, FVector const& HitPoint) override;
	UFUNCTION(NetMulticast, Unreliable)
	void Muticast_OnTakeDamage(FVector const& HitPoint, UParticleSystem* HitEmitter);
	#pragma endregion IFPSCharacterInterface

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCameraComponent* DeathCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* FPSArmMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UHealthComponent* HealthComponent;

	/// <summary>
	/// This array is the same size as EWeaponType.
	/// Index 0 - None
	/// Index 1 - MainWeapon
	/// Index 2 - SubWeapon
	/// </summary>
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	TArray<AFPSWeaponBase*> CurrentWeapons;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	TArray<AFPSWeaponBase*> StartWeapons;
	
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	AFPSWeaponBase* CurrentlyHeldWeapon;

	// TODO: Maybe use TMap for all the weapons below?
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AFPSWeaponBase> M4A1Class;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AFPSWeaponBase> PistolClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AFPSWeaponBase> AK47Class;

	// Hit Effect
	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* HitEmitterOnTakeDamage;

	// Cache
	UPROPERTY()
	UFPSGameInstance* FPSGameInstance;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Pickup & Equip Weapon
	void PickupWeapon(AFPSWeaponBase* const& WeaponToPickup);
	void PlayEquipAnim(AFPSWeaponBase* const& WeaponToEquip);
	UFUNCTION(Server, Reliable)
	void Server_EquipWeapon(AFPSWeaponBase* const& WeaponToEquip);

	// Crouch
	FTimerHandle CrouchTimerHandle;
	FVector CameraRelativeLocation_Default;
	UPROPERTY(EditDefaultsOnly)
	FVector CameraRelativeLocationOnCrouch = FVector::ZeroVector;
	FVector DesiredCameraRelativeLocation;
	UFUNCTION()
	void CrouchSimulate();

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
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnDeath();
	void HandleCameraOnDeath();
	
	UFUNCTION()
	void OnSpawn();
	UFUNCTION(Client, Reliable)
	void Client_WeaponSetupOnSpawn();
	UFUNCTION(Server, Reliable)
	void Server_WeaponSetupOnSpawn(FUserData const& UserData);
	void HandleCameraOnSpawn();
#pragma endregion Spawn & Death
};

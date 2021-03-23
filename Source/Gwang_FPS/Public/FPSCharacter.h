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

UCLASS()
class GWANG_FPS_API AFPSCharacter : public ACharacter, public IFPSCharacterInterface
{
	GENERATED_BODY()

public:
	AFPSCharacter();

	//////////////////
	// Input bindings
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

	void Pickup();

	void Drop();
	UFUNCTION(Server, Reliable)
	void Server_DropWeapon(AFPSWeaponBase* Weapon);

	void Reload();
	UFUNCTION(Server, Reliable)
	void Server_Reload(AFPSWeaponBase* Weapon);

	DECLARE_DELEGATE_OneParam(FOneBooleanDelegate, bool)
	void HandleGameStatusWidget(bool bDisplay);
	// Input bindings
	//////////////////

	//////////////////////////
	// IFPSCharacterInterface
	FTransform GetCameraTransform_Implementation() override;
	USkeletalMeshComponent* GetCharacterMesh_Implementation() override;
	USkeletalMeshComponent* GetArmMesh_Implementation() override;
	// IFPSCharacterInterface
	//////////////////////////

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* CameraContainer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* FPSArmMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* HandCollider;

	TWeakObjectPtr<AFPSWeaponBase> CurrentFocus;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AFPSWeaponBase* CurrentWeapon;

	float RespawnDelay = 5.f;

	// Cache
	UPROPERTY()
	USkeletalMeshComponent* FPSCharacterMesh;
	UPROPERTY()
	UCapsuleComponent* CharacterCapsuleComponent;
	FVector DefaultCameraRelativeLocation;
	FTransform DefaultCharacterMeshRelativeTransform;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBeginOverlapHandCollider(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlapHandCollider(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void EquipWeapon(AFPSWeaponBase* Weapon);
	UFUNCTION(Server, Reliable)
	void Server_EquipWeapon(AFPSWeaponBase* Weapon);

	/////////////////
	// Health & Death
	UFUNCTION()
	void OnDamageReceived();

	UFUNCTION()
	void OnHealthAcquired();
		
	UFUNCTION()
	void OnDeath();

	void RespawnPlayer();

	void CollisionHandleOnDeath();
	void CollisionHandleOnRespawn();
	// Health & Death
	/////////////////
};

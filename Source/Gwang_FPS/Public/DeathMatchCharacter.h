// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DeathMatchCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class AWeaponBase;

UCLASS()
class GWANG_FPS_API ADeathMatchCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADeathMatchCharacter();

	USkeletalMeshComponent* GetArmMesh() const { return ArmMesh; }
	FVector GetCameraLocation() const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	TSubclassOf<AWeaponBase> RifleClass;

	UPROPERTY(VisibleAnywhere, Category = Weapon)
	AWeaponBase* CurrentWeapon;

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* ArmMesh;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;


protected:
	virtual void BeginPlay();

	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	// Weapon
	void OnBeginFire();
	UFUNCTION(Server, Reliable)
	void Server_OnBeginFire();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnBeginFire();

	void OnEndFire();
	UFUNCTION(Server, Reliable)
	void Server_OnEndFire();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnEndFire();


	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPSCharacterInterface.h"
#include "FPSCharacter.generated.h"

class UCameraComponent;
class USkeletalMeshComponent;

UCLASS()
class GWANG_FPS_API AFPSCharacter : public ACharacter, public IFPSCharacterInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFPSCharacter();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	// IFPSCharacterInterface
	bool HasWeapon_Implementation(EWeaponType WeaponType) override;
	void EquipWeapon_Implementation(EWeaponType WeaponType) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = FPSCharacter, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = FPSCharacter, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FPSArms;

	TMap<EWeaponType, int16> Weapons;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};

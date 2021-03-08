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

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Pickup();

	bool HasWeapon(EWeaponType WeaponType);
	void PickupWeapon(EWeaponType WeaponType);
	void EquipWeapon(AFPSWeaponBase* Weapon);

	UFUNCTION(Server, Reliable)
	void Server_EquipWeapon(AFPSWeaponBase* Weapon);

	// IFPSCharacterInterface
	void OnBeginOverlapWeapon_Implementation(AFPSWeaponBase* Weapon) override;
	void OnEndOverlapWeapon_Implementation() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* FPSArms;

	TMap<EWeaponType, int16> Weapons;

	AFPSWeaponBase* CurrentFocusWeapon;
	FTimerHandle PickupTraceTimerHandle;
	int NumOfOverlappingWeapons;

	// Temporary thing
	bool bHasAnyWeapons;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(Client, Unreliable)
	void Client_CheckForWeapon();

};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DeathMatchCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class AWeaponBase;
class ADeathMatchPlayerState;

UCLASS()
class GWANG_FPS_API ADeathMatchCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	ADeathMatchCharacter();

	USkeletalMeshComponent* GetArmMesh() const { return ArmMesh; }
	FVector GetCameraLocation() const;
	ADeathMatchPlayerState* GetPlayerState() const { return PlayerState; }

	// TakeDamage
	UFUNCTION(Server, Reliable)
	void Server_TakeDamage(float DamageOnHealth, float DamageOnArmor, AActor* DamageCauser);


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

	ADeathMatchPlayerState* PlayerState;

protected:
	virtual void BeginPlay();

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	void MoveForward(float Val);
	void MoveRight(float Val);

	// Possession
	void PossessedBy(AController* NewController) override;
	void UnPossessed() override;

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

	// Health
	UPROPERTY(Replicated)
	float Health = 100.f;

	// OnDeath
	UFUNCTION(Server, Reliable)
	void Server_OnDeath(AActor* DeathCauser);

	// OnKill
	UFUNCTION(Server, Reliable)
	void Server_OnKill(ADeathMatchCharacter* DeadPlayer);


};

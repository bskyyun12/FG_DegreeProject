// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponInterface.h"
#include "FPSGameInstance.h"
#include "GrenadeBase.generated.h"

class UBoxComponent;
class USphereComponent;
class ADeathMatchPlayerController;

USTRUCT(BlueprintType)
struct FTrajectory
{
	GENERATED_BODY()

	UPROPERTY()
	FVector LaunchLocation;
	UPROPERTY()
	FVector LaunchForward;
	UPROPERTY()
	float LaunchAngleInRad;
	UPROPERTY()
	float LaunchSpeed;
};

USTRUCT(BlueprintType)
struct FGrenadeInfo
{
	GENERATED_BODY()

	// Stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeaponType WeaponType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExplosionDamage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ArmorPenetration;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExplosionRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExplodeInSeconds;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ThrowingPower;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Bounceness;

	// SocketNames to Equip Weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FP_SocketName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TP_SocketName;

	// Explosion Effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystem* ExplosionEmitter;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* ExplosionSound;

	// AnimMontages
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* FP_ThrowingAnimation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* TP_ThrowingAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* FP_EquipAnim;

	FGrenadeInfo()
	{
		// Stats
		DisplayName = TEXT("Gweapon");
		WeaponType = EWeaponType::Grenade;
		ExplosionDamage = 200.f;
		ArmorPenetration = 0.5f;
		ExplosionRadius = 500.f;
		ExplodeInSeconds = 3.f;
		ThrowingPower = 3000.f;
		Bounceness = 0.5f;

		FP_SocketName = "Weapon_Rifle";
		TP_SocketName = "Weapon_Rifle";

		// Explosion Effect
		ExplosionEmitter = nullptr;
		ExplosionSound = nullptr;

		// AnimMontages
		FP_ThrowingAnimation = nullptr;
		TP_ThrowingAnimation = nullptr;
		FP_EquipAnim = nullptr;
	}
};

UCLASS()
class GWANG_FPS_API AGrenadeBase : public AActor, public IWeaponInterface
{
	GENERATED_BODY()
	
public:
	AGrenadeBase();

	// Temp
	FColor GetRoleColor();

	// Getters
	EWeaponType GetWeaponType_Implementation() const override;

	// Setters
	void SetVisibility_Implementation(bool NewVisibility) override;

	// Weapon Equip
	void OnWeaponEquipped_Implementation(ADeathMatchCharacter* NewOwner) override;

	// Fire
	void BeginFire_Implementation() override;
	void EndFire_Implementation() override;

protected:
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USceneComponent* Root;

	/** Weapon mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* FPWeaponMesh;

	/** Weapon mesh: 3rd person view (seen only by others) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* TPWeaponMesh;

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USphereComponent* ExplosionCollider;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGrenadeInfo GrenadeInfo;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FTrajectory Trajectory;

	float Gravity = 981.f;
	float DisplacementX;
	float DisplacementZ;
	FVector NewLocation;
	FVector PrevLocation;

	FTimerHandle PathDrawingtimer;

	UPROPERTY(Replicated)
	ADeathMatchCharacter* LatestOwner;

	bool bIsUsed;

	float FlightTime;

	UPROPERTY(ReplicatedUsing=OnRep_ServerExplosionTime)
	float ServerExplosionTime;
	UFUNCTION()
	void OnRep_ServerExplosionTime();
	float ClientExplosionTime;

protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void Tick(float DeltaSeconds) override;

	// Fire
	void Fire();
	UFUNCTION(Server, Reliable)
	void Server_Fire();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Fire();

	// Trajectory
	void InitTrajectory();
	void DrawGrenadePath();
	void SimulateTrajectory(const float& DeltaSeconds, bool bMoveMesh);

	// OnExplosion
	void Explode();
	void ExplosionEffects();
};

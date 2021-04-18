// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponInterface.h"
#include "FPSGameInstance.h"
#include "MeleeWeaponBase.generated.h"

class UBoxComponent;
class USphereComponent;

USTRUCT(BlueprintType)
struct FMeleeWeaponInfo
{
	GENERATED_BODY()

	// Stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeaponType WeaponType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ArmorPenetration;

	// SocketNames to Equip Weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FP_SocketName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TP_SocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* FP_EquipAnim;

	FMeleeWeaponInfo()
	{
		// Stats
		DisplayName = TEXT("MeleeWeapon");
		WeaponType = EWeaponType::Melee;
		Damage = 50.f;
		ArmorPenetration = 0.5f;

		FP_SocketName = "Weapon_Melee";
		TP_SocketName = "Weapon_Melee";

		// AnimMontages
		FP_EquipAnim = nullptr;
	}
};

UCLASS()
class GWANG_FPS_API AMeleeWeaponBase : public AActor, public IWeaponInterface
{
	GENERATED_BODY()
	
public:
	AMeleeWeaponBase();

	// Temp
	FColor GetRoleColor();

	// Getters
	EWeaponType GetWeaponType_Implementation() const override;

	// Setters
	void SetVisibility_Implementation(bool NewVisibility) override;

	// Weapon Equip
	void OnWeaponEquipped_Implementation(ADeathMatchCharacter* NewOwner) override;
	void OnWeaponDropped_Implementation() override;

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
	UBoxComponent* InteractCollider;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FMeleeWeaponInfo MeleeWeaponInfo;

protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};

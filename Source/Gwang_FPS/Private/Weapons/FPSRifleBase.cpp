// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/FPSRifleBase.h"
#include <Components/SceneComponent.h>
#include <DrawDebugHelpers.h>
#include <Kismet/GameplayStatics.h>
#include <PhysicalMaterials/PhysicalMaterial.h>

#include "Components/HealthComponent.h"
#include "FPSCharacter.h"
#include "AnimInstances/FPSAnimInterface.h"

void AFPSRifleBase::Client_OnFPWeaponEquipped_Implementation(AFPSCharacter* FPSCharacter)
{
	Super::Client_OnFPWeaponEquipped_Implementation(FPSCharacter);
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Client_OnFPWeaponEquipped_Implementation()"));

	if (!ensure(FPSCharacter != nullptr))
	{
		return;
	}

	if (ClientWeaponMesh != nullptr && FPSCharacter->GetArmMesh() != nullptr)
	{
		ClientWeaponMesh->AttachToComponent(FPSCharacter->GetArmMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FP_WeaponSocketName);

		UAnimInstance* FPSArmsAnim = FPSCharacter->GetArmMesh()->GetAnimInstance();
		if (FPSArmsAnim != nullptr)
		{
			if (UKismetSystemLibrary::DoesImplementInterface(FPSArmsAnim, UFPSAnimInterface::StaticClass()))
			{
				IFPSAnimInterface::Execute_UpdateBlendPose(FPSArmsAnim, 1);
			}
		}
	}
}

void AFPSRifleBase::Client_OnFPWeaponDroped_Implementation(AFPSCharacter* FPSCharacter)
{
	Super::Client_OnFPWeaponDroped_Implementation(FPSCharacter);
	UE_LOG(LogTemp, Warning, TEXT("AFPSRifleBase::Client_OnFPWeaponDroped_Implementation()"));
}

void AFPSRifleBase::Server_OnTPWeaponEquipped_Implementation(AFPSCharacter* FPSCharacter)
{
	Super::Server_OnTPWeaponEquipped_Implementation(FPSCharacter);
	UE_LOG(LogTemp, Warning, TEXT("AFPSRifleBase::Server_OnTPWeaponEquipped_Implementation()"));

	if (!ensure(FPSCharacter != nullptr))
	{
		return;
	}

	if (RepWeaponMesh != nullptr && FPSCharacter->GetCharacterMesh() != nullptr)
	{
		RepWeaponMesh->SetSimulatePhysics(false);
		RepWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		RepWeaponMesh->AttachToComponent(OwnerCharacter->GetCharacterMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TP_WeaponSocketName);
	}
}

void AFPSRifleBase::Server_OnTPWeaponDroped_Implementation(AFPSCharacter* FPSCharacter)
{
	Super::Server_OnTPWeaponDroped_Implementation(FPSCharacter);
	UE_LOG(LogTemp, Warning, TEXT("AFPSRifleBase::Server_OnTPWeaponDroped_Implementation()"));
}

void AFPSRifleBase::Server_OnBeginFireWeapon_Implementation(AFPSCharacter* FPSCharacter)
{
	Super::Server_OnBeginFireWeapon_Implementation(FPSCharacter);
	UE_LOG(LogTemp, Warning, TEXT("AFPSRifleBase::Server_OnBeginFireWeapon_Implementation"));

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	RifleFireDelegate = FTimerDelegate::CreateUObject(this, &AFPSRifleBase::Fire, FPSCharacter);
	World->GetTimerManager().SetTimer(RifleFireTimer, RifleFireDelegate, WeaponInfo.FireRate, true, 0.f);
	return;
}

void AFPSRifleBase::Server_OnEndFireWeapon_Implementation()
{
	Super::Server_OnEndFireWeapon_Implementation();
	UE_LOG(LogTemp, Warning, TEXT("AFPSRifleBase::Server_OnEndFireWeapon_Implementation"));
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	World->GetTimerManager().ClearTimer(RifleFireTimer);
}

void AFPSRifleBase::Fire(AFPSCharacter* FPSCharacter)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSRifleBase::Fire"));
	// Todo: check fire conditions. ammo, reloading etc..

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	Multicast_FireEffects();

	FHitResult Hit;
	FVector Start = FPSCharacter->GetCameraTransform().GetLocation();
	FVector End = Start + FPSCharacter->GetCameraTransform().GetRotation().GetForwardVector() * WeaponInfo.Range;
	DrawDebugLine(World, Start, End, FColor::Red, true);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());
	Params.bReturnPhysicalMaterial = true;
	bool bIsHit = World->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1, Params); // ECC_GameTraceChannel1 = DamageTrace
	if (bIsHit)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor != nullptr)
		{
			UHealthComponent* HealthComp = Cast<UHealthComponent>(HitActor->GetComponentByClass(UHealthComponent::StaticClass()));
			if (HealthComp != nullptr && !HealthComp->IsDead())
			{
				float Damage = CalcDamageToApply(Hit.PhysMaterial.Get());
				HealthComp->AddHealth(-Damage);
				UE_LOG(LogTemp, Warning, TEXT("Damage Taken: %f, Attacker: %s, Damaged Actor: %s"), Damage, *GetOwner()->GetName(), *HitActor->GetName());
			}
		}

		DrawDebugPoint(World, Hit.ImpactPoint, 10.f, FColor::Green, true);
	}
}

void AFPSRifleBase::Multicast_FireEffects_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSRifleBase::Multicast_FireEffects_Implementation"));
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	if (FireEmitter != nullptr && ClientWeaponMesh != nullptr)
	{
		bool bIsLocalPlayer = UGameplayStatics::GetPlayerPawn(World, 0) == GetOwner();
		USkeletalMeshComponent* MeshToSpawnEffects = bIsLocalPlayer ? ClientWeaponMesh : RepWeaponMesh;
		FName MuzzleSocketName = bIsLocalPlayer ? FP_MuzzleSocketName : TP_MuzzleSocketName;
		UGameplayStatics::SpawnEmitterAttached(FireEmitter, MeshToSpawnEffects, MuzzleSocketName);

		if (FireSound != nullptr)
		{
			UGameplayStatics::SpawnSoundAttached(FireSound, MeshToSpawnEffects, MuzzleSocketName);
		}
	}
}

float AFPSRifleBase::CalcDamageToApply(const UPhysicalMaterial* PhysMat)
{
	float DamageMultiplier = 1.f;

	EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(PhysMat);
	switch (SurfaceType)
	{
	case SurfaceType_Default:
		break;
	case SurfaceType1:	// Head
		DamageMultiplier = 5.f;
		break;
	case SurfaceType2:	// Torso
		DamageMultiplier = 4.f;
		break;
	case SurfaceType3:	// Arms
		DamageMultiplier = 1.f;
		break;
	case SurfaceType4:	// Legs
		DamageMultiplier = 2.f;
		break;
	case SurfaceType5:	// Pelvis
		DamageMultiplier = 3.f;
		break;
	}

	return WeaponInfo.Damage * DamageMultiplier;
}

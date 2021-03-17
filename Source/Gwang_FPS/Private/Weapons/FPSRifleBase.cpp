// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/FPSRifleBase.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#include "Components/HealthComponent.h"
#include "FPSCharacter.h"
#include "AnimInstances/FPSAnimInterface.h"
#include "FPSPlayerControllerInterface.h"

void AFPSRifleBase::Client_OnFPWeaponEquipped_Implementation(AFPSCharacter* FPSCharacter)
{
	Super::Client_OnFPWeaponEquipped_Implementation(FPSCharacter);
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Client_OnFPWeaponEquipped_Implementation()"));

	if (FPSCharacter == nullptr)
	{
		return;
	}

	if (FPWeaponMesh != nullptr && FPSCharacter->GetArmMesh() != nullptr)
	{
		FPWeaponMesh->AttachToComponent(FPSCharacter->GetArmMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FP_WeaponSocketName);

		if (WeaponInfo.EquipAnim != nullptr)
		{
			FPSCharacter->GetArmMesh()->PlayAnimation(WeaponInfo.EquipAnim, false);
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

	if (TPWeaponMesh != nullptr && FPSCharacter->GetCharacterMesh() != nullptr)
	{
		TPWeaponMesh->SetSimulatePhysics(false);
		TPWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		TPWeaponMesh->AttachToComponent(OwnerCharacter->GetCharacterMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TP_WeaponSocketName);
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
	World->GetTimerManager().SetTimer(ServerRifleFireTimer, RifleFireDelegate, WeaponInfo.FireRate, true, 0.f);
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
	World->GetTimerManager().ClearTimer(ServerRifleFireTimer);
}

void AFPSRifleBase::Fire(AFPSCharacter* FPSCharacter)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSRifleBase::Fire"));

	if (CanFire() == false)
	{
		Server_OnEndFireWeapon();
		return;
	}

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
				HealthComp->Server_AddHealth(-Damage);
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

	bool bIsLocalPlayer = UGameplayStatics::GetPlayerPawn(World, 0) == GetOwner();
	if (bIsLocalPlayer == false)
	{
		PlayFireEmitter(false);
		PlayFireSound(false);
	}
}

void AFPSRifleBase::Client_OnBeginFireWeapon_Implementation()
{
	Super::Client_OnBeginFireWeapon_Implementation();
	UE_LOG(LogTemp, Warning, TEXT("AFPSRifleBase::Client_OnBeginFireWeapon_Implementation"));

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	World->GetTimerManager().SetTimer(ClientRifleFireTimer, this, &AFPSRifleBase::Client_FireEffects, WeaponInfo.FireRate, true, 0.f);
}

void AFPSRifleBase::Client_FireEffects()
{
	if (CanFire() == false)
	{
		// TODO: play no ammo sound and anim?
		Client_OnEndFireWeapon();
		return;
	}

	PlayFireEmitter(true);
	PlayFireSound(true);
	ShakeCamera();
	Recoil();
}

void AFPSRifleBase::Client_OnEndFireWeapon_Implementation()
{
	Super::Client_OnEndFireWeapon_Implementation();
	UE_LOG(LogTemp, Warning, TEXT("AFPSRifleBase::Client_OnEndFireWeapon_Implementation"));

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	World->GetTimerManager().ClearTimer(ClientRifleFireTimer);
	RecoilTimer = 0.f;
}

void AFPSRifleBase::PlayFireEmitter(bool FPWeapon)
{
	if (FPWeaponMesh != nullptr)
	{
		if (FireEmitter != nullptr)
		{
			if (FPWeapon)
			{
				UGameplayStatics::SpawnEmitterAttached(FireEmitter, FPWeaponMesh, FP_MuzzleSocketName);
			}
			else
			{
				UGameplayStatics::SpawnEmitterAttached(FireEmitter, TPWeaponMesh, TP_MuzzleSocketName);
			}
		}
	}
}

void AFPSRifleBase::PlayFireSound(bool FPWeapon)
{
	if (FPWeaponMesh != nullptr)
	{
		if (FireSound != nullptr)
		{
			if (FPWeapon)
			{
				UGameplayStatics::SpawnSoundAttached(FireSound, FPWeaponMesh, FP_MuzzleSocketName);
			}
			else
			{
				UGameplayStatics::SpawnSoundAttached(FireSound, TPWeaponMesh, TP_MuzzleSocketName);
			}
		}
	}
}

void AFPSRifleBase::ShakeCamera()
{
	AController* InstigatorController = GetInstigatorController();
	if (InstigatorController != nullptr && CameraShakeOnFire != nullptr)
	{
		if (UKismetSystemLibrary::DoesImplementInterface(InstigatorController, UFPSPlayerControllerInterface::StaticClass()))
		{
			IFPSPlayerControllerInterface::Execute_ShakeCamera(InstigatorController, CameraShakeOnFire);
		}
	}
}

void AFPSRifleBase::Recoil()
{
	AController* InstigatorController = GetInstigatorController();
	if (InstigatorController != nullptr)
	{
		if (UKismetSystemLibrary::DoesImplementInterface(InstigatorController, UFPSPlayerControllerInterface::StaticClass()))
		{
			float PitchDelta = 0.f;
			float YawDelta = 0.f;
			if (RecoilCurve_Vertical != nullptr && RecoilCurve_Horizontal != nullptr)
			{
				PitchDelta = RecoilCurve_Vertical->GetFloatValue(RecoilTimer);
				YawDelta = RecoilCurve_Horizontal->GetFloatValue(RecoilTimer);
				RecoilTimer += WeaponInfo.FireRate;
			}
			IFPSPlayerControllerInterface::Execute_AddControlRotation(InstigatorController, FRotator(PitchDelta, YawDelta, 0.f));
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

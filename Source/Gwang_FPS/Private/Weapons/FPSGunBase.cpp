// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/FPSGunBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#include "Components/HealthComponent.h"
#include "FPSCharacterInterface.h"
#include "FPSPlayerControllerInterface.h"

void AFPSGunBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSGunBase, bHasAmmoClip);
	DOREPLIFETIME(AFPSGunBase, CurrentAmmo);
}

void AFPSGunBase::Server_Fire_Implementation()
{
	--CurrentAmmo;	// Super::Server_Fire_Implementation() will call CanFire() so 

	Super::Server_Fire_Implementation();
	UE_LOG(LogTemp, Warning, TEXT("AFPSGunBase::Server_Fire_Implementation"));

	if (GetOwner() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetOwner(), UFPSCharacterInterface::StaticClass()))
	{
		// TODO: In VR, start should be rear sight, end should be front sight
		FHitResult Hit;
		FTransform CameraTransform = IFPSCharacterInterface::Execute_GetCameraTransform(GetOwner());
		FVector Start = CameraTransform.GetLocation();
		FVector End = Start + CameraTransform.GetRotation().GetForwardVector() * WeaponInfo.Range;

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		Params.AddIgnoredActor(GetOwner());
		Params.bReturnPhysicalMaterial = true;

		UWorld* World = GetWorld();
		if (!ensure(World != nullptr))
		{
			return;
		}
		bool bIsHit = World->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1, Params); // ECC_GameTraceChannel1 = DamageTrace
		if (bIsHit)
		{
			if (Hit.GetActor() != nullptr && UKismetSystemLibrary::DoesImplementInterface(Hit.GetActor(), UFPSCharacterInterface::StaticClass()))
			{
				if (GetInstigatorController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetInstigatorController(), UFPSPlayerControllerInterface::StaticClass()))
				{
					IFPSPlayerControllerInterface::Execute_OnApplyDamage(GetInstigatorController());
				}

				float Damage = CalcDamageToApply(Hit.PhysMaterial.Get());
				FPointDamageEvent DamageEvent;
				//DamageEvent.ShotDirection = (Hit.ImpactPoint - GetActorLocation()).GetSafeNormal();
				Hit.GetActor()->TakeDamage(Damage, DamageEvent, GetInstigatorController(), GetOwner());
			}
		}
	}
}

void AFPSGunBase::Client_FireEffects_Implementation()
{
	Super::Client_FireEffects_Implementation();
	UE_LOG(LogTemp, Warning, TEXT("AFPSGunBase::Client_FireEffects_Implementation"));

	ShakeCamera();
	Recoil();
}

void AFPSGunBase::Client_OnEndFireWeapon_Implementation()
{
	Super::Client_OnEndFireWeapon_Implementation();
	UE_LOG(LogTemp, Warning, TEXT("AFPSGunBase::Client_OnEndFireWeapon_Implementation"));

	RecoilTimer = 0.f;
}

void AFPSGunBase::ShakeCamera()
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

void AFPSGunBase::Recoil()
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

void AFPSGunBase::EquipAmmoClip()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSGunBase::EquipAmmoClip"));
}

void AFPSGunBase::RemoveAmmoClip()
{
	bHasAmmoClip = false;
}

bool AFPSGunBase::CanFire()
{
	if (Super::CanFire() == false)
	{
		return false;
	}

	return GetOwner() != nullptr && bHasAmmoClip && CurrentAmmo > 0;
}

void AFPSGunBase::Client_Fire_Implementation()
{
	Super::Client_Fire_Implementation();
}

void AFPSGunBase::Client_Reload_Implementation()
{
	Super::Client_Reload_Implementation();
	UE_LOG(LogTemp, Warning, TEXT("AFPSGunBase::Client_Reload_Implementation"));
}

void AFPSGunBase::Server_Reload_Implementation()
{
	Super::Server_Reload_Implementation();
	bHasAmmoClip = true;
	CurrentAmmo = MaxAmmo;
}

void AFPSGunBase::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (GetOwner() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetOwner(), UFPSCharacterInterface::StaticClass()))
	{
		USkeletalMeshComponent* CharacterMesh = IFPSCharacterInterface::Execute_GetCharacterMesh(GetOwner());
		TPWeaponMesh->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponInfo.TP_CharacterSocketName);
	}
}

float AFPSGunBase::CalcDamageToApply(const UPhysicalMaterial* PhysMat)
{
	float DamageMultiplier = 1.f;

	EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(PhysMat);
	switch (SurfaceType)
	{
	case SurfaceType_Default:
		break;
	case SurfaceType1:	// Head
		DamageMultiplier = 10.f;
		break;
	case SurfaceType2:	// Torso
		DamageMultiplier = 3.f;
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

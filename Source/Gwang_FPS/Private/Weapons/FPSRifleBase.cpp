// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/FPSRifleBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#include "Components/HealthComponent.h"
#include "FPSCharacterInterface.h"
#include "FPSPlayerControllerInterface.h"

void AFPSRifleBase::Client_OnFPWeaponEquipped_Implementation(AFPSCharacter* OwnerCharacter)
{
	Super::Client_OnFPWeaponEquipped_Implementation(OwnerCharacter);
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Client_OnFPWeaponEquipped_Implementation()"));
}

void AFPSRifleBase::Client_OnFPWeaponDroped_Implementation()
{
	Super::Client_OnFPWeaponDroped_Implementation();
	UE_LOG(LogTemp, Warning, TEXT("AFPSRifleBase::Client_OnFPWeaponDroped_Implementation()"));
}

void AFPSRifleBase::Server_OnTPWeaponEquipped_Implementation(AFPSCharacter* OwnerCharacter)
{
	Super::Server_OnTPWeaponEquipped_Implementation(OwnerCharacter);
	UE_LOG(LogTemp, Warning, TEXT("AFPSRifleBase::Server_OnTPWeaponEquipped_Implementation()"));
}

void AFPSRifleBase::Server_OnTPWeaponDroped_Implementation()
{
	Super::Server_OnTPWeaponDroped_Implementation();
	UE_LOG(LogTemp, Warning, TEXT("AFPSRifleBase::Server_OnTPWeaponDroped_Implementation()"));
}

void AFPSRifleBase::Server_OnBeginFireWeapon_Implementation()
{
	Super::Server_OnBeginFireWeapon_Implementation();
	UE_LOG(LogTemp, Warning, TEXT("AFPSRifleBase::Server_OnBeginFireWeapon_Implementation"));
}

void AFPSRifleBase::Server_OnEndFireWeapon_Implementation()
{
	Super::Server_OnEndFireWeapon_Implementation();
	UE_LOG(LogTemp, Warning, TEXT("AFPSRifleBase::Server_OnEndFireWeapon_Implementation"));
}

void AFPSRifleBase::Client_OnBeginFireWeapon_Implementation()
{
	Super::Client_OnBeginFireWeapon_Implementation();
	UE_LOG(LogTemp, Warning, TEXT("AFPSRifleBase::Client_OnBeginFireWeapon_Implementation"));
}

void AFPSRifleBase::Client_OnEndFireWeapon_Implementation()
{
	Super::Client_OnEndFireWeapon_Implementation();
	UE_LOG(LogTemp, Warning, TEXT("AFPSRifleBase::Client_OnEndFireWeapon_Implementation"));

	RecoilTimer = 0.f;
}

void AFPSRifleBase::Server_Fire_Implementation()
{
	Super::Server_Fire_Implementation();
	UE_LOG(LogTemp, Warning, TEXT("AFPSRifleBase::Server_Fire_Implementation"));

	if (GetOwner() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetOwner(), UFPSCharacterInterface::StaticClass()))
	{
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
		}
	}
}

void AFPSRifleBase::Multicast_FireEffects_Implementation()
{
	Super::Multicast_FireEffects_Implementation();
	UE_LOG(LogTemp, Warning, TEXT("AFPSRifleBase::Multicast_FireEffects_Implementation"));

}

void AFPSRifleBase::Client_Fire_Implementation()
{
	Super::Client_Fire_Implementation();
	UE_LOG(LogTemp, Warning, TEXT("AFPSRifleBase::Client_Fire_Implementation"));

}

void AFPSRifleBase::Client_FireEffects_Implementation()
{
	Super::Client_FireEffects_Implementation();
	UE_LOG(LogTemp, Warning, TEXT("AFPSRifleBase::Client_FireEffects_Implementation"));

	ShakeCamera();
	Recoil();
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

void AFPSRifleBase::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (GetOwner() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetOwner(), UFPSCharacterInterface::StaticClass()))
	{
		USkeletalMeshComponent* CharacterMesh = IFPSCharacterInterface::Execute_GetCharacterMesh(GetOwner());
		TPWeaponMesh->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponInfo.TP_CharacterSocketName);
	}
}

void AFPSRifleBase::Client_Reload_Implementation()
{
	Super::Client_Reload_Implementation();
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

// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/FPSRifleBase.h"
#include <PhysicalMaterials/PhysicalMaterial.h>
#include <DrawDebugHelpers.h>

#include "Components/HealthComponent.h"
#include "FPSCharacter.h"

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
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

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

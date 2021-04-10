// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/GunBase.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#include "DeathMatchCharacter.h"

// Temp
#include "DrawDebugHelpers.h"

AGunBase::AGunBase()
{

}

void AGunBase::OnBeginFire()
{
	Super::OnBeginFire();

	if (CanFire() == false)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	if (WeaponInfo.bIsAutomatic)
	{
		World->GetTimerManager().SetTimer(FireTimer, this, &AGunBase::Fire, WeaponInfo.FireRate, true, 0.f);
	}
	else
	{
		if (bCooldown == false)
		{
			Fire();
			bCooldown = true;
			if (World->GetTimerManager().IsTimerActive(CooldownTimer) == false)
			{
				World->GetTimerManager().SetTimer(CooldownTimer, [&]()
					{
						bCooldown = false;

					}, WeaponInfo.FireRate, false);
			}
		}
	}
}

void AGunBase::Fire()
{
	Super::Fire();

	if (CanFire() == false)
	{
		return;
	}

	if (CurrentOwner != nullptr)
	{
		UWorld* World = GetWorld();
		if (World != nullptr)
		{
			FHitResult Hit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);
			Params.AddIgnoredActor(CurrentOwner);
			Params.bReturnPhysicalMaterial = true;

			const FVector Start = CurrentOwner->GetCameraLocation();
			const FVector End = CurrentOwner->GetCameraLocation() + CurrentOwner->GetActorForwardVector() * WeaponInfo.Range;

			DrawDebugLine(World, Start, End, GetRoleColor(), false, 1.f);

			if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
			{
				DrawDebugPoint(World, Hit.ImpactPoint, 20.f, FColor::Red, false, 1.f);

				ADeathMatchCharacter* HitPlayer = Cast<ADeathMatchCharacter>(Hit.GetActor());
				if (HitPlayer != nullptr)
				{
					// TODO: Hit effect	(t.e. blood)

					bool bIsPlayerHost = CurrentOwner->GetLocalRole() == ROLE_Authority && CurrentOwner->IsLocallyControlled();
					if (CurrentOwner->GetLocalRole() == ROLE_AutonomousProxy || bIsPlayerHost)
					{
						// TODO: Crosshair UI change					
					}

					if (CurrentOwner->GetLocalRole() == ROLE_Authority)
					{
						float DamageOnHealth = 0.f;
						float DamageOnArmor = 0.f;
						CalcDamageToApply(Hit.PhysMaterial.Get(), DamageOnHealth, DamageOnArmor);
						HitPlayer->Server_TakeDamage(DamageOnHealth, DamageOnArmor, CurrentOwner);
					}
				}
				else
				{
					// TODO: Hit effect on environment
				}
			}
		}
	}
}

void AGunBase::OnEndFire()
{
	Super::OnEndFire();

	if (WeaponInfo.bIsAutomatic)
	{
		UWorld* World = GetWorld();
		if (!ensure(World != nullptr))
		{
			return;
		}
		World->GetTimerManager().ClearTimer(FireTimer);
	}
}

bool AGunBase::CanFire()
{
	if (!Super::CanFire())
	{
		return false;
	}

	return true;
}

void AGunBase::CalcDamageToApply(const UPhysicalMaterial* PhysMat, float& DamageOnHealth, float& DamageOnArmor)
{
	float DamageMultiplier = 1.f;

	EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(PhysMat);
	switch (SurfaceType)
	{
	case SurfaceType_Default:
		break;
	case SurfaceType1:	// Head
		DamageMultiplier = 3.5f;
		break;
	case SurfaceType2:	// Torso
		DamageMultiplier = 1.f;
		break;
	case SurfaceType3:	// Arms
		DamageMultiplier = 0.5f;
		break;
	case SurfaceType4:	// Legs
		DamageMultiplier = 0.5f;
		break;
	case SurfaceType5:	// Pelvis
		DamageMultiplier = 1.f;
		break;
	}

	DamageOnHealth = WeaponInfo.Damage * DamageMultiplier * WeaponInfo.ArmorPenetration;
	DamageOnArmor = WeaponInfo.Damage * DamageMultiplier * (1 - WeaponInfo.ArmorPenetration);
}

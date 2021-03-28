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
	DOREPLIFETIME(AFPSGunBase, CurrentAmmo);
}

void AFPSGunBase::Server_OnBeginFireWeapon_Implementation()
{
	//Super::Server_OnBeginFireWeapon_Implementation();

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	if (WeaponInfo.bIsAutomatic)
	{
		World->GetTimerManager().SetTimer(ServerAutomaticFireTimer, this, &AFPSGunBase::Server_Fire, WeaponInfo.FireRate, true, 0.f);
	}
	else
	{
		Server_Fire();
		WeaponInfo.bCanFire = false;
		if (World->GetTimerManager().IsTimerActive(ServerFireCoolDownTimer) == false)
		{
			World->GetTimerManager().SetTimer(ServerFireCoolDownTimer, [&]()
				{
					WeaponInfo.bCanFire = true;

				}, WeaponInfo.FireRate, false);
		}
	}
}

void AFPSGunBase::Server_Fire_Implementation()
{
	Super::Server_Fire_Implementation();
	UE_LOG(LogTemp, Warning, TEXT("AFPSGunBase::Server_Fire_Implementation"));

	if (CanFire() == false)
	{
		return;
	}
	--CurrentAmmo;	// TODO: Doing this before Super::Server_Fire_Implementation() because the ammo kept going -1 on automatic fire

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
			if (Hit.GetActor() != nullptr && UKismetSystemLibrary::DoesImplementInterface(Hit.GetActor(), UFPSCharacterInterface::StaticClass()))
			{
				float DamageOnHealth = 0.f;
				float DamageOnArmor = 0.f;
				CalcDamageToApply(Hit.PhysMaterial.Get(), DamageOnHealth, DamageOnArmor);
				IFPSCharacterInterface::Execute_TakeDamage(Hit.GetActor(), GetOwner(), DamageOnHealth, DamageOnArmor);

				if (GetInstigatorController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetInstigatorController(), UFPSPlayerControllerInterface::StaticClass()))
				{
					IFPSPlayerControllerInterface::Execute_OnApplyDamage(GetInstigatorController());
				}
			}
		}
	}
}

void AFPSGunBase::Server_OnEndFireWeapon_Implementation()
{
	Super::Server_OnEndFireWeapon_Implementation();

	if (WeaponInfo.bIsAutomatic)
	{
		UWorld* World = GetWorld();
		if (!ensure(World != nullptr))
		{
			return;
		}
		World->GetTimerManager().ClearTimer(ServerAutomaticFireTimer);
	}
}

void AFPSGunBase::Client_OnBeginFireWeapon_Implementation()
{
	//Super::Client_OnBeginFireWeapon_Implementation();

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	if (WeaponInfo.bIsAutomatic)
	{
		World->GetTimerManager().SetTimer(ClientAutomaticFireTimer, this, &AFPSGunBase::Client_Fire, WeaponInfo.FireRate, true, 0.f);
	}
	else
	{
		Client_Fire();
		WeaponInfo.bCanFire = false;
		if (World->GetTimerManager().IsTimerActive(ServerFireCoolDownTimer) == false)
		{
			World->GetTimerManager().SetTimer(ServerFireCoolDownTimer, [&]()
				{
					WeaponInfo.bCanFire = true;

				}, WeaponInfo.FireRate, false);
		}
	}
}

void AFPSGunBase::Client_Fire_Implementation()
{
	Super::Client_Fire_Implementation();
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

	if (WeaponInfo.bIsAutomatic)
	{
		UWorld* World = GetWorld();
		if (!ensure(World != nullptr))
		{
			return;
		}
		World->GetTimerManager().ClearTimer(ClientAutomaticFireTimer);
	}

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

bool AFPSGunBase::CanFire()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Server_Fire_Implementation"));
	UE_LOG(LogTemp, Warning, TEXT("CurrentAmmo: %i"), CurrentAmmo);
	if (Super::CanFire() == false)
	{
		return false;
	}

	if (WeaponInfo.bIsAutomatic == false)
	{

	}

	return CurrentAmmo > 0;
}

void AFPSGunBase::Client_Reload_Implementation()
{
	Super::Client_Reload_Implementation();
	UE_LOG(LogTemp, Warning, TEXT("AFPSGunBase::Client_Reload_Implementation"));
}

void AFPSGunBase::Server_Reload_Implementation()
{
	Super::Server_Reload_Implementation();
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

void AFPSGunBase::CalcDamageToApply(const UPhysicalMaterial* PhysMat, float& DamageOnHealth, float& DamageOnArmor)
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
// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/FPSGunBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#include "Components/HealthComponent.h"
#include "FPSCharacterInterface.h"
#include "FPSPlayerControllerInterface.h"

void AFPSGunBase::BeginPlay()
{
	Super::BeginPlay();

	OnReset();
}

void AFPSGunBase::OnReset()
{
	Super::OnReset();
	CurrentAmmo = MagazineCapacity;	// OnRep_CurrentAmmo()
	CurrentRemainingAmmo = RemainingAmmo;
	Client_UpdateAmmoUI(CurrentAmmo, CurrentRemainingAmmo);
}

void AFPSGunBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSGunBase, CurrentAmmo);
	DOREPLIFETIME(AFPSGunBase, CurrentRemainingAmmo);
}

void AFPSGunBase::HandleWeaponEquip()
{
	Super::HandleWeaponEquip();
	Client_UpdateAmmoUI(CurrentAmmo, CurrentRemainingAmmo);
}

void AFPSGunBase::Server_OnBeginFireWeapon_Implementation()
{
	// Super::Server_OnBeginFireWeapon_Implementation();

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
		World->GetTimerManager().SetTimer(FireTimer_Server, this, &AFPSGunBase::Server_Fire, WeaponInfo.FireRate, true, 0.f);
	}
	else
	{
		if (bCooldown_Server == false)
		{
			Server_Fire();
			bCooldown_Server = true;
			if (World->GetTimerManager().IsTimerActive(CooldownTimer_Server) == false)
			{
				World->GetTimerManager().SetTimer(CooldownTimer_Server, [&]()
					{
						bCooldown_Server = false;

					}, WeaponInfo.FireRate, false);
			}
		}
	}
}

void AFPSGunBase::Server_Fire_Implementation()
{
	Super::Server_Fire_Implementation();

	if (CanFire() == false)
	{
		return;
	}

	CurrentAmmo--;	// OnRep_CurrentAmmo()
	Client_UpdateAmmoUI(CurrentAmmo, CurrentRemainingAmmo);

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
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))	// ECC_GameTraceChannel1 = DamageTrace
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor != nullptr)
			{
				bool bIsHitActorPlayer = UKismetSystemLibrary::DoesImplementInterface(HitActor, UFPSCharacterInterface::StaticClass());
				if (bIsHitActorPlayer)
				{
					float DamageOnHealth = 0.f;
					float DamageOnArmor = 0.f;
					CalcDamageToApply(Hit.PhysMaterial.Get(), DamageOnHealth, DamageOnArmor);
					IFPSCharacterInterface::Execute_TakeDamage(HitActor, GetOwner(), DamageOnHealth, DamageOnArmor, Hit.ImpactPoint);

					if (GetInstigatorController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetInstigatorController(), UFPSPlayerControllerInterface::StaticClass()))
					{
						// This is for shooter(damage causer). Currently used for changing crosshair's appearance on successful hit.
						IFPSPlayerControllerInterface::Execute_OnApplyDamage(GetInstigatorController());
					}
				}
				else
				{
					Multicast_HitEffect(Hit.ImpactPoint, WeaponInfo.HitEmitter);
				}
			}
		}
	}
}

void AFPSGunBase::Multicast_HitEffect_Implementation(FVector const& HitPoint, UParticleSystem* HitEmitter)
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	if (WeaponInfo.HitEmitter != nullptr)
	{
		UGameplayStatics::SpawnEmitterAtLocation(World, HitEmitter, HitPoint);
	}
}


void AFPSGunBase::Client_UpdateAmmoUI_Implementation(int _CurrentAmmo, int _CurrentRemainingAmmo)
{
	if (GetInstigatorController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetInstigatorController(), UFPSPlayerControllerInterface::StaticClass()))
	{
		IFPSPlayerControllerInterface::Execute_OnUpdateAmmoUI(GetInstigatorController(), _CurrentAmmo, _CurrentRemainingAmmo);
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
		World->GetTimerManager().ClearTimer(FireTimer_Server);
	}
}

bool AFPSGunBase::CanFire()
{
	if (Super::CanFire() == false)
	{
		return false;
	}
	return CurrentAmmo > 0;
}

bool AFPSGunBase::CanReload()
{
	if (Super::CanReload() == false)
	{
		return false;
	}
	return CurrentRemainingAmmo > 0 && CurrentAmmo != MagazineCapacity;
}

void AFPSGunBase::Client_OnReload_Implementation()
{
	Super::Client_OnReload_Implementation();
}

void AFPSGunBase::Server_OnEndReload_Implementation()
{
	Super::Server_OnEndReload_Implementation();

	int AmmoToPool = MagazineCapacity - CurrentAmmo;
	AmmoToPool = (CurrentRemainingAmmo < AmmoToPool) ? CurrentRemainingAmmo : AmmoToPool;
	CurrentRemainingAmmo -= AmmoToPool;
	CurrentAmmo += AmmoToPool;	// OnRep_CurrentAmmo()

	Client_UpdateAmmoUI(CurrentAmmo, CurrentRemainingAmmo);
}


void AFPSGunBase::OnRep_CurrentAmmo()
{
	Client_UpdateAmmoUI(CurrentAmmo, CurrentRemainingAmmo);
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
		World->GetTimerManager().SetTimer(FireTimer_Client, this, &AFPSGunBase::Client_Fire, WeaponInfo.FireRate, true, 0.f);
	}
	else
	{
		if (bCooldown_Client == false)
		{
			Client_Fire();
			bCooldown_Client = true;
			if (World->GetTimerManager().IsTimerActive(CooldownTimer_Client) == false)
			{
				World->GetTimerManager().SetTimer(CooldownTimer_Client, [&]()
					{
						bCooldown_Client = false;

					}, WeaponInfo.FireRate, false);
			}
		}
	}
}

void AFPSGunBase::Client_FireEffects_Implementation()
{
	Super::Client_FireEffects_Implementation();

	ShakeCamera();
	Recoil();
}

void AFPSGunBase::Client_OnEndFireWeapon_Implementation()
{
	Super::Client_OnEndFireWeapon_Implementation();

	if (WeaponInfo.bIsAutomatic)
	{
		UWorld* World = GetWorld();
		if (!ensure(World != nullptr))
		{
			return;
		}
		World->GetTimerManager().ClearTimer(FireTimer_Client);
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
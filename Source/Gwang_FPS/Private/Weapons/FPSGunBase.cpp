// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/FPSGunBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#include "Components/HealthComponent.h"
#include "FPSCharacterInterface.h"
#include "FPSPlayerControllerInterface.h"

void AFPSGunBase::BeginPlay()
{
	Super::BeginPlay();
}

void AFPSGunBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSGunBase, CurrentAmmo);
	DOREPLIFETIME(AFPSGunBase, RemainingAmmo);
}

void AFPSGunBase::Server_OnWeaponEquipped_Implementation(AFPSCharacter* OwnerCharacter)
{
	Super::Server_OnWeaponEquipped_Implementation(OwnerCharacter);

	Client_UpdateAmmoUI(CurrentAmmo, RemainingAmmo);
}

void AFPSGunBase::OnRep_Owner()
{
	Super::OnRep_Owner();

	Client_UpdateAmmoUI(CurrentAmmo, RemainingAmmo);
}

void AFPSGunBase::Server_OnBeginFireWeapon_Implementation()
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}	
	
	if (CanFire() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("(ServerBeginFire) CanFire() == false"));
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
	UE_LOG(LogTemp, Warning, TEXT("AFPSGunBase::Server_Fire_Implementation"));

	if (CanFire() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("CanFire() == false"));
		return;
	}

	CurrentAmmo--;	// OnRep_CurrentAmmo()
	Client_UpdateAmmoUI(CurrentAmmo, RemainingAmmo);

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

void AFPSGunBase::Client_UpdateAmmoUI_Implementation(int _CurrentAmmo, int _RemainingAmmo)
{
	if (GetInstigatorController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetInstigatorController(), UFPSPlayerControllerInterface::StaticClass()))
	{
		IFPSPlayerControllerInterface::Execute_OnUpdateAmmoUI(GetInstigatorController(), _CurrentAmmo, _RemainingAmmo);
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
	return RemainingAmmo > 0;
}

void AFPSGunBase::Client_OnReload_Implementation()
{
	Super::Client_OnReload_Implementation();
	UE_LOG(LogTemp, Warning, TEXT("AFPSGunBase::Client_Reload_Implementation"));
}

void AFPSGunBase::Server_OnEndReload_Implementation()
{
	Super::Server_OnEndReload_Implementation();
	
	int AmmoToPool = MagazineCapacity - CurrentAmmo;
	AmmoToPool = (RemainingAmmo < AmmoToPool) ? RemainingAmmo : AmmoToPool;
	RemainingAmmo -= AmmoToPool;	
	CurrentAmmo += AmmoToPool;	// OnRep_CurrentAmmo()

	Client_UpdateAmmoUI(CurrentAmmo, RemainingAmmo);
}


void AFPSGunBase::OnRep_CurrentAmmo()
{
	Client_UpdateAmmoUI(CurrentAmmo, RemainingAmmo);
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
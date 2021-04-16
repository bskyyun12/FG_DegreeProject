// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/GunBase.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#include "DeathMatchCharacter.h"
#include "PlayerControllerInterface.h"
#include "Animation/FPSAnimInterface.h"
#include "DeathMatchPlayerController.h"

// Temp
#include "DrawDebugHelpers.h"

// Temp
FColor AGunBase::GetRoleColor()
{
	if (GetCurrentOwner()->GetLocalRole() == ROLE_Authority)
	{
		return FColor::Red;
	}

	if (GetCurrentOwner()->GetLocalRole() == ROLE_AutonomousProxy)
	{
		return FColor::Green;
	}

	if (GetCurrentOwner()->GetLocalRole() == ROLE_SimulatedProxy)
	{
		return FColor::Blue;
	}

	return FColor::Cyan;
}

AGunBase::AGunBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;	// Otherwise, RPC calls are ignored

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Create a 1st person weapon mesh component
	FPWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPWeaponMesh"));
	FPWeaponMesh->SetOnlyOwnerSee(true);
	FPWeaponMesh->bCastDynamicShadow = false;
	FPWeaponMesh->CastShadow = false;
	FPWeaponMesh->SetCollisionProfileName(TEXT("NoCollision"));
	FPWeaponMesh->SetupAttachment(RootComponent);

	// Create a 3rd person weapon mesh component
	TPWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TPWeaponMesh"));
	TPWeaponMesh->SetOwnerNoSee(true);
	TPWeaponMesh->SetCollisionProfileName(TEXT("NoCollision"));
	TPWeaponMesh->SetupAttachment(RootComponent);

	InteractCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractCollider"));
	InteractCollider->SetCollisionProfileName(TEXT("NoCollision"));
	InteractCollider->SetupAttachment(TPWeaponMesh);
}

void AGunBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGunBase, CurrentAmmo);
	DOREPLIFETIME(AGunBase, CurrentRemainingAmmo);
}

void AGunBase::BeginPlay()
{
	Super::BeginPlay();

	CurrentAmmo = WeaponInfo.MagazineCapacity;
	CurrentRemainingAmmo = WeaponInfo.RemainingAmmo;
}

#pragma region Getters & Setters
ADeathMatchCharacter* AGunBase::GetCurrentOwner()
{
	if (GetOwner() == nullptr)
	{
		return nullptr;
	}

	return Cast<ADeathMatchCharacter>(GetOwner());
}

ADeathMatchPlayerController* AGunBase::GetOwnerController()
{
	if (GetCurrentOwner() == nullptr)
	{
		return nullptr;
	}

	if (GetCurrentOwner()->GetController() == nullptr)
	{
		return nullptr;
	}

	return Cast<ADeathMatchPlayerController>(GetCurrentOwner()->GetController());
}

EWeaponType AGunBase::GetWeaponType_Implementation() const
{
	return WeaponInfo.WeaponType;
}

void AGunBase::SetVisibility_Implementation(bool NewVisibility)
{
	FPWeaponMesh->SetVisibility(NewVisibility);
	TPWeaponMesh->SetVisibility(NewVisibility);
}
#pragma endregion Getters & Setters

#pragma region Weapon Equip & Drop
void AGunBase::OnWeaponEquipped_Implementation(ADeathMatchCharacter* NewOwner)
{
	UE_LOG(LogTemp, Warning, TEXT("GunBase::OnWeaponEquipped => NewOwner(%s)'s role: %i. / Weapon(%s)'s role: %i."), *NewOwner->GetName(), NewOwner->GetLocalRole(), *GetName(), GetLocalRole());
	if (!ensure(NewOwner != nullptr))
	{
		return;
	}

	SetOwner(NewOwner);
	SetInstigator(NewOwner);

	if (GetCurrentOwner()->IsLocallyControlled())
	{
		UpdateAmmoUI(CurrentAmmo, CurrentRemainingAmmo);
	}

	// skeleton is not yet created in the constructor, so AttachToComponent should be happened after constructor
	TPWeaponMesh->SetSimulatePhysics(false);
	TPWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TPWeaponMesh->AttachToComponent(NewOwner->GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), WeaponInfo.TP_SocketName);

	FPWeaponMesh->AttachToComponent(NewOwner->GetArmMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), WeaponInfo.FP_SocketName);

	InteractCollider->SetCollisionProfileName(TEXT("NoCollision"));

	if (GetCurrentOwner()->IsLocallyControlled())
	{
		// Play FP_EquipAnim
		UAnimInstance* AnimInstance = GetCurrentOwner()->GetArmMesh()->GetAnimInstance();
		if (AnimInstance != nullptr && WeaponInfo.FP_EquipAnim != nullptr)
		{
			AnimInstance->Montage_Play(WeaponInfo.FP_EquipAnim);
		}
	}
	else
	{
		// TODO: Play TP_EquipAnim
	}
}

void AGunBase::OnWeaponDropped_Implementation()
{
	if (GetCurrentOwner() != nullptr)
	{
		InteractCollider->SetCollisionProfileName(TEXT("Weapon_Dropped"));

		FPWeaponMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

		TPWeaponMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		TPWeaponMesh->SetCollisionProfileName(TEXT("Weapon_Dropped"));
		TPWeaponMesh->SetSimulatePhysics(true);

		if (GetCurrentOwner()->IsLocallyControlled())
		{
			UpdateAmmoUI(0, 0);
		}

		SetOwner(nullptr);
		SetInstigator(nullptr);
	}
}
#pragma endregion Weapon Equip & Drop

#pragma region Weapon Fire
void AGunBase::BeginFire_Implementation()
{
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
		if (bFireCooldown == false)
		{
			Fire();
			bFireCooldown = true;
			if (World->GetTimerManager().IsTimerActive(CooldownTimer) == false)
			{
				World->GetTimerManager().SetTimer(CooldownTimer, [&]()
					{
						bFireCooldown = false;

					}, WeaponInfo.FireRate, false);
			}
		}
	}
}

void AGunBase::Fire()
{
	if (CanFire() == false)
	{
		return;
	}

	// Fire Effects
	if (GetCurrentOwner()->IsLocallyControlled())
	{
		FireEffects();
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		Multicast_FireEffects();
	}

	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_FireEffects();
	}

	// Ammo
	CurrentAmmo--;
	if (GetCurrentOwner()->IsLocallyControlled())
	{
		UpdateAmmoUI(CurrentAmmo, CurrentRemainingAmmo);
	}

	FHitResult Hit;
	if (FireLineTrace(Hit))
	{
		// TODO: Change to interface approach
		ADeathMatchCharacter* HitPlayer = Cast<ADeathMatchCharacter>(Hit.GetActor());
		if (HitPlayer != nullptr)
		{
			// Attacker => Crosshair UI change on hit player
			if (GetCurrentOwner()->IsLocallyControlled())
			{
				if (GetCurrentOwner()->GetController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetCurrentOwner()->GetController(), UPlayerControllerInterface::StaticClass()))
				{
					IPlayerControllerInterface::Execute_ChangeCrosshairUIOnHit(GetCurrentOwner()->GetController());
				}
			}

			// Apply damage ( ROLE_Authority )
			if (GetCurrentOwner()->GetLocalRole() == ROLE_Authority)
			{
				float DamageOnHealth = 0.f;
				float DamageOnArmor = 0.f;
				CalcDamageToApply(Hit.PhysMaterial.Get(), DamageOnHealth, DamageOnArmor);
				HitPlayer->Server_TakeDamage((uint8)DamageOnHealth, (uint8)DamageOnArmor, GetOwner());
			}
		}
		else
		{
			if (GetCurrentOwner()->IsLocallyControlled())
			{
				ImpactEffect(Hit.ImpactPoint);
			}

			if (GetLocalRole() == ROLE_Authority)
			{
				Multicast_ImpactEffect(Hit.ImpactPoint);
			}

			if (GetLocalRole() == ROLE_AutonomousProxy)
			{
				Server_ImpactEffect(Hit.ImpactPoint);
			}
		}
	}
}

void AGunBase::EndFire_Implementation()
{
	if (WeaponInfo.bIsAutomatic)
	{
		UWorld* World = GetWorld();
		if (!ensure(World != nullptr))
		{
			return;
		}
		World->GetTimerManager().ClearTimer(FireTimer);
	}

	// Reset Recoil
	RecoilTimer = 0.f;
}

bool AGunBase::CanFire()
{
	return GetCurrentOwner() != nullptr && CurrentAmmo > 0 && !bIsReloading;
}

bool AGunBase::FireLineTrace(FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return false;
	}

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetCurrentOwner());
	Params.bReturnPhysicalMaterial = true;

	const FVector Start = GetCurrentOwner()->GetCameraLocation();
	const FVector End = GetCurrentOwner()->GetCameraLocation() + GetCurrentOwner()->GetCameraForward() * WeaponInfo.Range;

	DrawDebugLine(World, Start, End, GetRoleColor(), false, 1.f);

	if (World->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params))
	{
		DrawDebugPoint(World, OutHit.ImpactPoint, 20.f, FColor::Red, false, 1.f);
		return true;
	}

	return false;
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

void AGunBase::UpdateAmmoUI(const int& InCurrentAmmo, const int& InRemainingAmmo)
{
	if (GetCurrentOwner()->IsLocallyControlled())
	{
		if (GetOwnerController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetOwnerController(), UPlayerControllerInterface::StaticClass()))
		{
			IPlayerControllerInterface::Execute_UpdateWeaponUI(GetOwnerController(), WeaponInfo.DisplayName, InCurrentAmmo, InRemainingAmmo);
		}
	}
}
#pragma endregion Weapon Fire

#pragma region Impact Effect
void AGunBase::ImpactEffect(const FVector& ImpactPoint)
{
	if (WeaponInfo.HitEmitterOnEnvironment != nullptr)
	{
		UWorld* World = GetWorld();
		if (World != nullptr)
		{
			UGameplayStatics::SpawnEmitterAtLocation(World, WeaponInfo.HitEmitterOnEnvironment, ImpactPoint);
		}
	}
}

void AGunBase::Server_ImpactEffect_Implementation(const FVector& ImpactPoint)
{
	Multicast_ImpactEffect(ImpactPoint);
}

void AGunBase::Multicast_ImpactEffect_Implementation(const FVector& ImpactPoint)
{
	if (GetCurrentOwner() != nullptr && !GetCurrentOwner()->IsLocallyControlled())
	{
		ImpactEffect(ImpactPoint);
	}
}
#pragma endregion Impact Effect

#pragma region Fire Effects (FireSound, MuzzleFlash, Recoil, FireAnim ...)
void AGunBase::Server_FireEffects_Implementation()
{
	Multicast_FireEffects();
}

void AGunBase::Multicast_FireEffects_Implementation()
{
	if (GetCurrentOwner() != nullptr && !GetCurrentOwner()->IsLocallyControlled())
	{
		FireEffects();
	}
}

void AGunBase::FireEffects()
{
	if (GetCurrentOwner() != nullptr)
	{
		// This moves camera's pitch. 
		Recoil();

		// Play FireSound
		if (WeaponInfo.FireSound != nullptr)
		{
			UGameplayStatics::SpawnSoundAttached(WeaponInfo.FireSound, TPWeaponMesh, WeaponInfo.TP_FireEmitterSocketName);
			UE_LOG(LogTemp, Warning, TEXT("FireEffects => PlaySound ( %i )"), GetLocalRole());
		}

		// Locally controlled owner
		if (GetCurrentOwner()->IsLocallyControlled())
		{
			// FP FireAnim
			UAnimInstance* AnimInstance = GetCurrentOwner()->GetArmMesh()->GetAnimInstance();
			if (AnimInstance != nullptr && WeaponInfo.FP_FireAnimation != nullptr)
			{
				AnimInstance->Montage_Play(WeaponInfo.FP_FireAnimation);
			}

			// FP FireEmitter
			if (WeaponInfo.FireEmitter != nullptr)
			{
				UGameplayStatics::SpawnEmitterAttached(WeaponInfo.FireEmitter, FPWeaponMesh, WeaponInfo.FP_FireEmitterSocketName);
				UE_LOG(LogTemp, Warning, TEXT("FireEffects => FP_FireEmitter ( %i )"), GetLocalRole());
			}

			// Camera shake
			GetOwnerController()->ClientStartCameraShake(WeaponInfo.CameraShakeOnFire);
		}
		else
		{
			// TP FireAnim
			UAnimInstance* AnimInstance = GetCurrentOwner()->GetMesh()->GetAnimInstance();
			if (AnimInstance != nullptr && WeaponInfo.TP_FireAnimation != nullptr)
			{
				AnimInstance->Montage_Play(WeaponInfo.TP_FireAnimation);
			}

			// TP FireEmitter
			if (WeaponInfo.FireEmitter != nullptr)
			{
				UGameplayStatics::SpawnEmitterAttached(WeaponInfo.FireEmitter, TPWeaponMesh, WeaponInfo.TP_FireEmitterSocketName);
				UE_LOG(LogTemp, Warning, TEXT("FireEffects => TP_FireEmitter ( %i )"), GetLocalRole());
			}
		}
	}
}

void AGunBase::Recoil()
{
	if (GetOwnerController() != nullptr)
	{
		// Camera pitch movement
		float PitchDelta = 0.f;
		if (WeaponInfo.RecoilCurve_Vertical != nullptr)
		{
			PitchDelta = WeaponInfo.RecoilCurve_Vertical->GetFloatValue(RecoilTimer);
			RecoilTimer += WeaponInfo.FireRate;
		}
		GetOwnerController()->SetControlRotation(GetOwnerController()->GetControlRotation() + FRotator(PitchDelta, 0.f, 0.f));
		GetCurrentOwner()->SetCameraWorldRotation(GetOwnerController()->GetControlRotation());
	}
}
#pragma endregion Fire Effects (FireSound, MuzzleFlash, Recoil, FireAnim ...)

#pragma region Weapon Reload
bool AGunBase::CanReload()
{
	return GetCurrentOwner() != nullptr && !bIsReloading && CurrentRemainingAmmo > 0 && CurrentAmmo != WeaponInfo.MagazineCapacity;
}

void AGunBase::BeginReload_Implementation()
{
	if (GetCurrentOwner() != nullptr && GetCurrentOwner()->IsLocallyControlled())
	{
		Reload();
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		Multicast_Reload();
	}

	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_Reload();
	}
}

void AGunBase::Server_Reload_Implementation()
{
	Multicast_Reload();
}

void AGunBase::Multicast_Reload_Implementation()
{
	if (GetCurrentOwner() != nullptr && !GetCurrentOwner()->IsLocallyControlled())
	{
		Reload();
	}
}

void AGunBase::Reload()
{
	if (CanReload())
	{
		Execute_EndFire(this);

		UWorld* World = GetWorld();
		if (!ensure(World != nullptr))
		{
			return;
		}

		bIsReloading = true;
		World->GetTimerManager().SetTimer(ReloadTimer, this, &AGunBase::OnEndReload, WeaponInfo.ReloadTime, false);

		if (GetCurrentOwner()->IsLocallyControlled())
		{
			// Play FP_ArmsReloadAnim
			UAnimInstance* AnimInstance = GetCurrentOwner()->GetArmMesh()->GetAnimInstance();
			if (AnimInstance != nullptr && WeaponInfo.FP_ArmsReloadAnim != nullptr)
			{
				AnimInstance->Montage_Play(WeaponInfo.FP_ArmsReloadAnim);
				UE_LOG(LogTemp, Warning, TEXT("Reload => FP_ArmsReloadAnim ( %i )"), GetLocalRole());
			}

			// Play FP_WeaponReloadAnim
			if (WeaponInfo.FP_WeaponReloadAnim != nullptr)
			{
				FPWeaponMesh->PlayAnimation(WeaponInfo.FP_WeaponReloadAnim, false);
			}
		}
		else
		{
			// Play TP_ReloadAnim
			UAnimInstance* AnimInstance = GetCurrentOwner()->GetMesh()->GetAnimInstance();
			if (AnimInstance != nullptr && WeaponInfo.TP_ReloadAnim != nullptr)
			{
				AnimInstance->Montage_Play(WeaponInfo.TP_ReloadAnim);
				UE_LOG(LogTemp, Warning, TEXT("Reload => TP_ReloadAnim ( %i )"), GetLocalRole());
			}
		}
	}
}

void AGunBase::OnEndReload()
{
	UE_LOG(LogTemp, Warning, TEXT("Reload => OnEndReload ( %i )"), GetLocalRole());

	bIsReloading = false;
	int AmmoToPool = WeaponInfo.MagazineCapacity - CurrentAmmo;
	AmmoToPool = (CurrentRemainingAmmo < AmmoToPool) ? CurrentRemainingAmmo : AmmoToPool;
	CurrentRemainingAmmo -= AmmoToPool;
	CurrentAmmo += AmmoToPool;

	if (GetCurrentOwner()->IsLocallyControlled())
	{
		UpdateAmmoUI(CurrentAmmo, CurrentRemainingAmmo);
	}
}
#pragma endregion Weapon Reload
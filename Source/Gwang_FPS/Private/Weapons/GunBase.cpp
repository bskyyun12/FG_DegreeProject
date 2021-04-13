// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/GunBase.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#include "DeathMatchCharacter.h"
#include "PlayerControllerInterface.h"

// Temp
#include "DrawDebugHelpers.h"
#include "Animation/FPSAnimInterface.h"

// Temp
FColor AGunBase::GetRoleColor()
{
	if (CurrentOwner->GetLocalRole() == ROLE_Authority)
	{
		UE_LOG(LogTemp, Warning, TEXT("ROLE_Authority"));
		return FColor::Red;
	}

	if (CurrentOwner->GetLocalRole() == ROLE_AutonomousProxy)
	{
		UE_LOG(LogTemp, Warning, TEXT("ROLE_AutonomousProxy"));
		return FColor::Green;
	}

	if (CurrentOwner->GetLocalRole() == ROLE_SimulatedProxy)
	{
		UE_LOG(LogTemp, Warning, TEXT("ROLE_SimulatedProxy"));
		return FColor::Blue;
	}

	return FColor::Cyan;
}

void AGunBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGunBase, CurrentAmmo);
	DOREPLIFETIME(AGunBase, CurrentRemainingAmmo);
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

void AGunBase::BeginPlay()
{
	Super::BeginPlay();

	CurrentAmmo = WeaponInfo.MagazineCapacity;
	CurrentRemainingAmmo = WeaponInfo.RemainingAmmo;
}

void AGunBase::OnWeaponEquipped_Implementation(ADeathMatchCharacter* NewOwner)
{
	UE_LOG(LogTemp, Warning, TEXT("GunBase::OnWeaponEquipped => NewOwner(%s)'s role: %i. / Weapon(%s)'s role: %i."), *NewOwner->GetName(), NewOwner->GetLocalRole(), *GetName(), GetLocalRole());
	if (!ensure(NewOwner != nullptr))
	{
		return;
	}

	if (NewOwner->IsLocallyControlled())
	{
		SetOwner(NewOwner);	// OnRep_Owner()
		SetInstigator(NewOwner);
	}

	CurrentOwner = NewOwner;
	if (CurrentOwner->IsLocallyControlled())
	{
		UpdateAmmoUI(CurrentAmmo, CurrentRemainingAmmo);
	}

	// skeleton is not yet created in the constructor, so AttachToComponent should be happened after constructor
	TPWeaponMesh->SetSimulatePhysics(false);
	TPWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TPWeaponMesh->AttachToComponent(NewOwner->GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), WeaponInfo.TP_SocketName);

	FPWeaponMesh->AttachToComponent(NewOwner->GetArmMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), WeaponInfo.FP_SocketName);

	InteractCollider->SetCollisionProfileName(TEXT("NoCollision"));

	// TODO: Notify Arm mesh to change animations
	//UAnimInstance* ArmsAnimInstance = CurrentOwner->GetArmMesh()->GetAnimInstance();
	//if (ArmsAnimInstance != nullptr && UKismetSystemLibrary::DoesImplementInterface(ArmsAnimInstance, UFPSAnimInterface::StaticClass()))
	//{
	//	IFPSAnimInterface::Execute_OnChangeWeapon(ArmsAnimInstance, WeaponInfo.WeaponType);
	//}
}

void AGunBase::OnWeaponDropped_Implementation()
{
	if (!ensure(CurrentOwner != nullptr))
	{
		return;
	}

	InteractCollider->SetCollisionProfileName(TEXT("Weapon_Dropped"));

	FPWeaponMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	TPWeaponMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	TPWeaponMesh->SetCollisionProfileName(TEXT("Weapon_Dropped"));
	TPWeaponMesh->SetSimulatePhysics(true);

	if (CurrentOwner->IsLocallyControlled())
	{
		UpdateAmmoUI(0, 0);
	}

	CurrentOwner = nullptr;
	SetOwner(nullptr);
	SetInstigator(nullptr);
}

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

	FireEffects();

	CurrentAmmo--;
	if (CurrentOwner->IsLocallyControlled())
	{
		UpdateAmmoUI(CurrentAmmo, CurrentRemainingAmmo);
	}

	if (CurrentOwner != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("GunBase::Fire => CurrentOwner(%s)'s role: %i. / Weapon's role: %i."), *CurrentOwner->GetName(), CurrentOwner->GetLocalRole(), GetLocalRole());

		FHitResult Hit;
		if (FireLineTrace(Hit))
		{
			// TODO: Change to interface approach
			ADeathMatchCharacter* HitPlayer = Cast<ADeathMatchCharacter>(Hit.GetActor());
			if (HitPlayer != nullptr)
			{
				// TODO: Hit effect	(t.e. blood)

				if (CurrentOwner->IsLocallyControlled())
				{
					// TODO: Crosshair UI change

					Recoil();
				}

				if (CurrentOwner->GetLocalRole() == ROLE_Authority)
				{
					float DamageOnHealth = 0.f;
					float DamageOnArmor = 0.f;
					CalcDamageToApply(Hit.PhysMaterial.Get(), DamageOnHealth, DamageOnArmor);
					HitPlayer->Server_TakeDamage((uint8)DamageOnHealth, (uint8)DamageOnArmor, CurrentOwner);
				}
			}
			else
			{
				// TODO: Move to NetMulticast Unreliable?
				//if (WeaponInfo.HitEmitterOnEnvironment != nullptr)
				//{
				//	UGameplayStatics::SpawnEmitterAtLocation(World, WeaponInfo.HitEmitterOnEnvironment, Hit.ImpactPoint);
				//}
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

bool AGunBase::CanFire() const
{
	return CurrentOwner != nullptr && CurrentAmmo > 0 && !bIsReloading;
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
	Params.AddIgnoredActor(CurrentOwner);
	Params.bReturnPhysicalMaterial = true;

	const FVector Start = CurrentOwner->GetCameraLocation();
	const FVector End = CurrentOwner->GetCameraLocation() + CurrentOwner->GetActorForwardVector() * WeaponInfo.Range;

	DrawDebugLine(World, Start, End, GetRoleColor(), false, 1.f);

	if (World->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params))
	{
		DrawDebugPoint(World, OutHit.ImpactPoint, 20.f, FColor::Red, false, 1.f);
		return true;
	}

	return false;
}

void AGunBase::FireEffects()
{
	// Play FireSound
	if (WeaponInfo.FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponInfo.FireSound, GetActorLocation());
	}

	if (CurrentOwner != nullptr)
	{
		// Locally controlled owner
		if (CurrentOwner->IsLocallyControlled())
		{
			// FP FireAnim
			UAnimInstance* AnimInstance = CurrentOwner->GetArmMesh()->GetAnimInstance();
			if (AnimInstance != nullptr && WeaponInfo.FP_FireAnimation != nullptr)
			{
				AnimInstance->Montage_Play(WeaponInfo.FP_FireAnimation);
			}

			// FP FireEmitter
			if (WeaponInfo.FireEmitter != nullptr)
			{
				UGameplayStatics::SpawnEmitterAttached(WeaponInfo.FireEmitter, FPWeaponMesh, WeaponInfo.FP_FireEmitterSocketName);
			}
		}
		// Not locally controlled
		else
		{
			// TP FireAnim
			UAnimInstance* AnimInstance = CurrentOwner->GetMesh()->GetAnimInstance();
			if (AnimInstance != nullptr && WeaponInfo.TP_FireAnimation != nullptr)
			{
				AnimInstance->Montage_Play(WeaponInfo.TP_FireAnimation);
			}

			// TP FireEmitter
			if (WeaponInfo.FireEmitter != nullptr)
			{
				UGameplayStatics::SpawnEmitterAttached(WeaponInfo.FireEmitter, TPWeaponMesh, WeaponInfo.TP_FireEmitterSocketName);
			}
		}
	}
}

void AGunBase::UpdateAmmoUI(const int& InCurrentAmmo, const int& InRemainingAmmo)
{
	if (CurrentOwner && CurrentOwner->IsLocallyControlled())
	{
		if (GetInstigatorController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetInstigatorController(), UPlayerControllerInterface::StaticClass()))
		{
			IPlayerControllerInterface::Execute_UpdateWeaponUI(GetInstigatorController(), WeaponInfo.DisplayName, InCurrentAmmo, InRemainingAmmo);
		}
	}
}

void AGunBase::SetVisibility_Implementation(bool NewVisibility)
{
	FPWeaponMesh->SetVisibility(NewVisibility);
	TPWeaponMesh->SetVisibility(NewVisibility);
}

bool AGunBase::CanReload()
{
	return CurrentOwner != nullptr && !bIsReloading;
}

void AGunBase::OnBeginReload()
{
	if (!CanReload())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	bIsReloading = true;
	World->GetTimerManager().SetTimer(ReloadTimer, this, &AGunBase::OnEndReload, WeaponInfo.ReloadTime, false);

	if (CurrentOwner->IsLocallyControlled())
	{
		// Play FP_ArmsReloadAnim
		UAnimInstance* AnimInstance = CurrentOwner->GetArmMesh()->GetAnimInstance();
		if (AnimInstance != nullptr && WeaponInfo.FP_ArmsReloadAnim != nullptr)
		{
			AnimInstance->Montage_Play(WeaponInfo.FP_ArmsReloadAnim);
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
		UAnimInstance* AnimInstance = CurrentOwner->GetMesh()->GetAnimInstance();
		if (AnimInstance != nullptr && WeaponInfo.TP_ReloadAnim != nullptr)
		{
			AnimInstance->Montage_Play(WeaponInfo.TP_ReloadAnim);
		}
	}

}

void AGunBase::OnEndReload()
{
	bIsReloading = false;
	int AmmoToPool = WeaponInfo.MagazineCapacity - CurrentAmmo;
	AmmoToPool = (CurrentRemainingAmmo < AmmoToPool) ? CurrentRemainingAmmo : AmmoToPool;
	CurrentRemainingAmmo -= AmmoToPool;
	CurrentAmmo += AmmoToPool;

	if (CurrentOwner->IsLocallyControlled())
	{
		UpdateAmmoUI(CurrentAmmo, CurrentRemainingAmmo);
	}
}

void AGunBase::Recoil()
{
	// TODO: Implement Recoil
	float PitchDelta = 0.f;
	if (WeaponInfo.RecoilCurve_Vertical != nullptr)
	{
		PitchDelta = WeaponInfo.RecoilCurve_Vertical->GetFloatValue(RecoilTimer);
		RecoilTimer += WeaponInfo.FireRate;
	}
	// PlayerController => SetControlRotation(GetControlRotation() + FRotator(PitchDelta, 0.f, 0.f));

	// TODO: Shake Camera?
	// PlayerController => ClientStartCameraShake(WeaponInfo.CameraShakeOnFire);
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

EWeaponType AGunBase::GetWeaponType_Implementation() const
{
	return WeaponInfo.WeaponType;
}
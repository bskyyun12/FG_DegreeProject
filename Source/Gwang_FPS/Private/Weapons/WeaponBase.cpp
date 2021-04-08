// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponBase.h"
#include "Kismet/GameplayStatics.h"

#include "DeathMatchCharacter.h"

// Temp
FColor AWeaponBase::GetRoleColor()
{
	if (CurrentOwner->GetLocalRole() == ROLE_Authority)
	{
		return FColor::Red;
	}

	if (CurrentOwner->GetLocalRole() == ROLE_AutonomousProxy)
	{
		return FColor::Green;
	}

	if (CurrentOwner->GetLocalRole() == ROLE_SimulatedProxy)
	{
		return FColor::Blue;
	}

	return FColor::Cyan;
}

AWeaponBase::AWeaponBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//bReplicates = true;

	// Create a weapon mesh component
	FPWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPWeaponMesh"));
	FPWeaponMesh->SetOnlyOwnerSee(true);
	FPWeaponMesh->bCastDynamicShadow = false;
	FPWeaponMesh->CastShadow = false;
	FPWeaponMesh->SetupAttachment(RootComponent);

	// Create a weapon mesh component
	TPWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TPWeaponMesh"));
	TPWeaponMesh->SetOwnerNoSee(true);
	FPWeaponMesh->SetupAttachment(RootComponent);
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

}

void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeaponBase::OnWeaponEquipped(ADeathMatchCharacter* NewOwner)
{
	UE_LOG(LogTemp, Warning, TEXT("AWeaponBase::OnWeaponEquipped"));

	SetOwner(NewOwner);
	SetInstigator(NewOwner);
	CurrentOwner = NewOwner;

	// skeleton is not yet created in the constructor, so this should be happened after constructor
	FPWeaponMesh->AttachToComponent(NewOwner->GetArmMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), WeaponInfo.FP_SocketName);
	TPWeaponMesh->AttachToComponent(NewOwner->GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), WeaponInfo.TP_SocketName);
}

bool AWeaponBase::CanFire()
{
	return GetOwner() != nullptr;
}

void AWeaponBase::OnBeginFire()
{
	if (CanFire())
	{
		Fire();
	}
}

void AWeaponBase::Fire()
{
	UE_LOG(LogTemp, Warning, TEXT("AWeaponBase::Fire"));

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

			if (FPWeaponMesh != nullptr)
			{
				// FP FireEmitter
				if (WeaponInfo.FireEmitter != nullptr)
				{
					UGameplayStatics::SpawnEmitterAttached(WeaponInfo.FireEmitter, FPWeaponMesh, WeaponInfo.FP_FireEmitterSocketName);
				}

				// FP FireSound
				if (WeaponInfo.FireSound != nullptr)
				{
					UGameplayStatics::SpawnSoundAttached(WeaponInfo.FireSound, FPWeaponMesh, WeaponInfo.FP_FireEmitterSocketName);
				}
			}
		}
		// Not locally controlled
		else
		{
			if (TPWeaponMesh != nullptr)
			{
				// TP FireEmitter
				if (WeaponInfo.FireEmitter != nullptr)
				{
					UGameplayStatics::SpawnEmitterAttached(WeaponInfo.FireEmitter, TPWeaponMesh, WeaponInfo.TP_FireEmitterSocketName);
				}

				// TP FireSound
				if (WeaponInfo.FireSound != nullptr)
				{
					UGameplayStatics::SpawnSoundAttached(WeaponInfo.FireSound, TPWeaponMesh, WeaponInfo.TP_FireEmitterSocketName);
				}
			}
		}
	}
}

void AWeaponBase::OnEndFire()
{
	UE_LOG(LogTemp, Warning, TEXT("AWeaponBase::OnEndFire"));
}

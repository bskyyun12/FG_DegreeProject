// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponBase.h"
#include "Kismet/GameplayStatics.h"

#include "DeathMatchCharacter.h"

// Temp
FColor AWeaponBase::GetRoleColor()
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

void AWeaponBase::Client_OnWeaponEquipped_Implementation(ADeathMatchCharacter* NewOwner)
{
	if (NewOwner->GetLocalRole() == ROLE_Authority)
	{
		UE_LOG(LogTemp, Warning, TEXT("(Server) AWeaponBase::OnWeaponEquipped"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("(Client) AWeaponBase::OnWeaponEquipped"));
	}

	SetOwner(NewOwner);	// OnRep_Owner()
	SetInstigator(NewOwner);
	CurrentOwner = NewOwner;

	// skeleton is not yet created in the constructor, so AttachToComponent should be happened after constructor
	TPWeaponMesh->SetSimulatePhysics(false);
	TPWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TPWeaponMesh->AttachToComponent(NewOwner->GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), WeaponInfo.TP_SocketName);

	FPWeaponMesh->AttachToComponent(NewOwner->GetArmMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), WeaponInfo.FP_SocketName);
}

void AWeaponBase::OnRep_Owner()
{
	Super::OnRep_Owner();

}

bool AWeaponBase::CanFire()
{
	return GetOwner() != nullptr;
}

// Here only do effects. Actual fire logic will be handled in children classes
void AWeaponBase::OnBeginFire() 
{
}

// Here only do effects. Actual fire logic will be handled in children classes
void AWeaponBase::Fire()
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

// Here only do effects. Actual fire logic will be handled in children classes
void AWeaponBase::OnEndFire()
{
}
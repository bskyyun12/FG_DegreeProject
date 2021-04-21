// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/MeleeWeaponBase.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

#include "DeathMatchCharacter.h"
#include "PlayerControllerInterface.h"

// Temp
#include "DrawDebugHelpers.h"

// Temp
FColor AMeleeWeaponBase::GetRoleColor()
{
	if (GetOwner() != nullptr)
	{
		if (GetOwner()->GetLocalRole() == ROLE_Authority)
		{
			return FColor::Red;
		}

		if (GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
		{
			return FColor::Green;
		}

		if (GetOwner()->GetLocalRole() == ROLE_SimulatedProxy)
		{
			return FColor::Blue;
		}
	}

	return FColor::Cyan;
}

AMeleeWeaponBase::AMeleeWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;	// Otherwise, RPC calls are ignored, and will fail to spawn properly in clients' side

	// Create a root component
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

void AMeleeWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

#pragma region Getters & Setters
EWeaponType AMeleeWeaponBase::GetWeaponType_Implementation() const
{
	return MeleeWeaponInfo.WeaponType;
}

void AMeleeWeaponBase::SetVisibility_Implementation(bool NewVisibility)
{
	FPWeaponMesh->SetVisibility(NewVisibility);
	TPWeaponMesh->SetVisibility(NewVisibility);
}
#pragma endregion Getters & Setters

#pragma region Weapon Equip
void AMeleeWeaponBase::OnWeaponEquipped_Implementation(ADeathMatchCharacter* NewOwner)
{
	UE_LOG(LogTemp, Warning, TEXT("GunBase::OnWeaponEquipped => NewOwner(%s)'s role: %i. / Weapon(%s)'s role: %i."), *NewOwner->GetName(), NewOwner->GetLocalRole(), *GetName(), GetLocalRole());
	if (!ensure(NewOwner != nullptr))
	{
		return;
	}

	SetOwner(NewOwner);
	SetInstigator(NewOwner);

	// skeleton is not yet created in the constructor, so AttachToComponent should be happened after constructor
	TPWeaponMesh->SetSimulatePhysics(false);
	TPWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TPWeaponMesh->AttachToComponent(NewOwner->GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), MeleeWeaponInfo.TP_SocketName);
	FPWeaponMesh->AttachToComponent(NewOwner->GetArmMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), MeleeWeaponInfo.FP_SocketName);
	InteractCollider->SetCollisionProfileName(TEXT("NoCollision"));

	if (NewOwner->IsLocallyControlled())
	{
		// Update Weapon UI
		if (NewOwner->GetController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(NewOwner->GetController(), UPlayerControllerInterface::StaticClass()))
		{
			IPlayerControllerInterface::Execute_UpdateWeaponUI(NewOwner->GetController(), MeleeWeaponInfo.DisplayName, 0, 0);
		}

		// Play FP_EquipAnim
		UAnimInstance* AnimInstance = NewOwner->GetArmMesh()->GetAnimInstance();
		if (AnimInstance != nullptr && MeleeWeaponInfo.FP_EquipAnim != nullptr)
		{
			AnimInstance->Montage_Play(MeleeWeaponInfo.FP_EquipAnim);
		}
	}
	else
	{
		//  TODO: Play TP_EquipAnim
	}
}

void AMeleeWeaponBase::OnWeaponDropped_Implementation()
{
	if (GetOwner() != nullptr)
	{
		InteractCollider->SetCollisionProfileName(TEXT("Weapon_Dropped"));

		FPWeaponMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

		TPWeaponMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		TPWeaponMesh->SetCollisionProfileName(TEXT("Weapon_Dropped"));
		TPWeaponMesh->SetSimulatePhysics(true);

		SetOwner(nullptr);
		SetInstigator(nullptr);
	}
}
#pragma endregion Weapon Equip
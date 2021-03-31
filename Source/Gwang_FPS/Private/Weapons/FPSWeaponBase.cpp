// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/FPSWeaponBase.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

#include "FPSCharacter.h"
#include "FPSCharacterInterface.h"
#include "FPSPlayerControllerInterface.h"

// Sets default values
AFPSWeaponBase::AFPSWeaponBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	this->SetRootComponent(RootComp);

	FPWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ClientWeapnMesh"));
	FPWeaponMesh->SetOnlyOwnerSee(true);
	FPWeaponMesh->SetupAttachment(RootComp);

	TPWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	TPWeaponMesh->SetOwnerNoSee(true);
	TPWeaponMesh->SetSimulatePhysics(true);
	TPWeaponMesh->SetCollisionProfileName("IgnoreCharacter");
	TPWeaponMesh->SetupAttachment(RootComp);

	InteractCollider = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	InteractCollider->SetSphereRadius(80.f);
	InteractCollider->SetupAttachment(TPWeaponMesh);

	bReplicates = true;
}

// Called when the game starts or when spawned
void AFPSWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

void AFPSWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSWeaponBase, bIsReloading);
}

void AFPSWeaponBase::Server_OnWeaponEquipped_Implementation(AFPSCharacter* OwnerCharacter)
{
	UE_LOG(LogTemp, Warning, TEXT("(Server)AFPSWeaponBase::OnWeaponEquipped"));
	UE_LOG(LogTemp, Warning, TEXT("(Server) Equip Weapon"));

	SetOwner(OwnerCharacter);	// OnRep_Owner()
	SetInstigator(OwnerCharacter);

	TPWeaponMesh->SetSimulatePhysics(false);
	TPWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InteractCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (UKismetSystemLibrary::DoesImplementInterface(GetOwner(), UFPSCharacterInterface::StaticClass()))
	{
		USkeletalMeshComponent* CharacterMesh = IFPSCharacterInterface::Execute_GetCharacterMesh(GetOwner());
		if (CharacterMesh != nullptr)
		{
			TPWeaponMesh->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponInfo.TP_CharacterSocketName);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("(Server_OnWeaponEquipped) CharacterMesh == nulllptr"));
		}

		USkeletalMeshComponent* ArmMesh = IFPSCharacterInterface::Execute_GetArmMesh(GetOwner());
		if (ArmMesh != nullptr)
		{
			FPWeaponMesh->AttachToComponent(ArmMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponInfo.FP_ArmsSocketName);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("(Server_OnWeaponEquipped) ArmMesh == nulllptr"));
		}
	}
}

void AFPSWeaponBase::Server_OnWeaponDroped_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("(Server)AFPSWeaponBase::OnWeaponDroped"));
	UE_LOG(LogTemp, Warning, TEXT("(Server) Drop Weapon"));

	FPWeaponMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	TPWeaponMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	InteractCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TPWeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TPWeaponMesh->SetSimulatePhysics(true);

	SetInstigator(nullptr);
	SetOwner(nullptr);	// OnRep_Owner()
}

void AFPSWeaponBase::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (GetOwner() != nullptr)	// OnEquip
	{
		UE_LOG(LogTemp, Warning, TEXT("(Client) Equip Weapon"));
		TPWeaponMesh->SetSimulatePhysics(false);
		TPWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		InteractCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if (UKismetSystemLibrary::DoesImplementInterface(GetOwner(), UFPSCharacterInterface::StaticClass()))
		{
			USkeletalMeshComponent* CharacterMesh = IFPSCharacterInterface::Execute_GetCharacterMesh(GetOwner());
			if (CharacterMesh != nullptr)
			{
				TPWeaponMesh->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponInfo.TP_CharacterSocketName);
			}

			USkeletalMeshComponent* ArmMesh = IFPSCharacterInterface::Execute_GetArmMesh(GetOwner());
			if (ArmMesh != nullptr)
			{
				FPWeaponMesh->AttachToComponent(ArmMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponInfo.FP_ArmsSocketName);
			}
		}
	}
	else // OnDrop
	{
		UE_LOG(LogTemp, Warning, TEXT("(Client) Drop Weapon"));
		FPWeaponMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		TPWeaponMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		InteractCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		TPWeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		TPWeaponMesh->SetSimulatePhysics(true);
	}
}

AFPSWeaponBase* AFPSWeaponBase::GetWeapon_Implementation()
{
	return this;
}

#pragma region Server Fire

FWeaponInfo AFPSWeaponBase::GetWeaponInfo()
{
	return WeaponInfo;
}

void AFPSWeaponBase::Server_OnBeginFireWeapon_Implementation()
{
	Server_Fire();
}

void AFPSWeaponBase::Server_Fire_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Server_Fire_Implementation"));
	if (CanFire() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("CanFire() == false"));
		Server_OnEndFireWeapon();
		return;
	}

	Multicast_FireEffects();
}

void AFPSWeaponBase::Multicast_FireEffects_Implementation()
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	bool bLocalControl = UGameplayStatics::GetPlayerPawn(World, 0) == GetOwner();
	if (bLocalControl == false)
	{
		PlayFireEmitter(false);
		PlayFireSound(false);
	}
	else if (bLocalControl && HasAuthority())
	{
		PlayFireEmitter(true);
		PlayFireSound(true);
	}
}

void AFPSWeaponBase::Server_OnEndFireWeapon_Implementation()
{
	
}
#pragma endregion

#pragma region Client Fire
void AFPSWeaponBase::Client_OnBeginFireWeapon_Implementation()
{
	Client_Fire();
}

void AFPSWeaponBase::Client_Fire_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Client_Fire_Implementation"));
	if (CanFire() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("CanFire() == false"));
		Client_OnEndFireWeapon();
		return;
	}

	Client_FireEffects();
}

void AFPSWeaponBase::Client_FireEffects_Implementation()
{
	if (!HasAuthority())	// Server's local fire effect is handled in Multicast_FireEffects_Implementation()
	{
		PlayFireEmitter(true);
		PlayFireSound(true);
	}
}

void AFPSWeaponBase::Client_OnEndFireWeapon_Implementation()
{
}

#pragma endregion

bool AFPSWeaponBase::CanFire()
{
	return GetOwner() != nullptr && !bIsReloading;
}

bool AFPSWeaponBase::CanReload()
{
	return !bIsReloading;
}

void AFPSWeaponBase::Server_OnReload_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Server_Reload_Implementation()"));

	bIsReloading = true;
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	if (World->GetTimerManager().IsTimerActive(ReloadTimer) == false)
	{
		World->GetTimerManager().SetTimer(ReloadTimer, this, &AFPSWeaponBase::Server_OnEndReload, WeaponInfo.ReloadTime, false);
	}
}

void AFPSWeaponBase::Client_OnReload_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Client_Reload_Implementation()"));

	bIsReloading = true;

	// Play FP_WeaponReloadAnim
	if (WeaponInfo.FP_WeaponReloadAnim != nullptr)
	{
		FPWeaponMesh->PlayAnimation(WeaponInfo.FP_WeaponReloadAnim, false);
	}
}

void AFPSWeaponBase::Server_OnEndReload_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Server_OnEndReload_Implementation"));
	bIsReloading = false;
}

void AFPSWeaponBase::PlayFireEmitter(bool FPWeapon)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::PlayFireEmitter"));
	if (FPWeaponMesh != nullptr)
	{
		if (WeaponInfo.FireEmitter != nullptr)
		{
			if (FPWeapon)
			{
				UE_LOG(LogTemp, Warning, TEXT("(FP Weapon) BANG!!"));
				UGameplayStatics::SpawnEmitterAttached(WeaponInfo.FireEmitter, FPWeaponMesh, WeaponInfo.FP_FireEmitterSocketName);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("(TP Weapon) BANG!!"));
				UGameplayStatics::SpawnEmitterAttached(WeaponInfo.FireEmitter, TPWeaponMesh, WeaponInfo.TP_FireEmitterSocketName);
			}
		}
	}
}

void AFPSWeaponBase::PlayFireSound(bool FPWeapon)
{
	if (FPWeaponMesh != nullptr)
	{
		if (WeaponInfo.FireSound != nullptr)
		{
			if (FPWeapon)
			{
				UGameplayStatics::SpawnSoundAttached(WeaponInfo.FireSound, FPWeaponMesh, WeaponInfo.FP_FireEmitterSocketName);
			}
			else
			{
				UGameplayStatics::SpawnSoundAttached(WeaponInfo.FireSound, TPWeaponMesh, WeaponInfo.TP_FireEmitterSocketName);
			}
		}
	}
}
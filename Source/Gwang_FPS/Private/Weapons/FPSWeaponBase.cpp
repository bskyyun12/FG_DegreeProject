// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/FPSWeaponBase.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
//#include "Net/UnrealNetwork.h" // GetLifetimeReplicatedProps

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

	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void AFPSWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

//void AFPSWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//	DOREPLIFETIME(AFPSWeaponBase, TPWeaponMesh);
//}

void AFPSWeaponBase::Client_OnFPWeaponEquipped_Implementation(AFPSCharacter* OwnerCharacter)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Client_OnFPWeaponEquipped_Implementation()"));

	if (OwnerCharacter != nullptr && UKismetSystemLibrary::DoesImplementInterface(OwnerCharacter, UFPSCharacterInterface::StaticClass()))
	{
		USkeletalMeshComponent* ArmMesh = IFPSCharacterInterface::Execute_GetArmMesh(OwnerCharacter);
		if (ArmMesh != nullptr)
		{
			FPWeaponMesh->AttachToComponent(ArmMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponInfo.FP_ArmsSocketName);

			UAnimInstance* AnimInstance = ArmMesh->GetAnimInstance();
			if (AnimInstance != nullptr)
			{
				if (WeaponInfo.FP_EquipAnim != nullptr)
				{ 
					AnimInstance->Montage_Play(WeaponInfo.FP_EquipAnim);
				}
			}
		}
	}
}

void AFPSWeaponBase::Client_OnFPWeaponDroped_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Client_OnFPWeaponDroped_Implementation()"));

	FPWeaponMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
}

void AFPSWeaponBase::Server_OnTPWeaponEquipped_Implementation(AFPSCharacter* OwnerCharacter)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Server_OnTPWeaponEquipped_Implementation()"));

	SetOwner(OwnerCharacter);
	SetInstigator(OwnerCharacter);

	TPWeaponMesh->SetSimulatePhysics(false);
	TPWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InteractCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (OwnerCharacter != nullptr && UKismetSystemLibrary::DoesImplementInterface(OwnerCharacter, UFPSCharacterInterface::StaticClass()))
	{
		USkeletalMeshComponent* CharacterMesh = IFPSCharacterInterface::Execute_GetCharacterMesh(OwnerCharacter);

		TPWeaponMesh->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponInfo.TP_CharacterSocketName);
	}
}

void AFPSWeaponBase::Server_OnTPWeaponDroped_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Server_OnTPWeaponDroped_Implementation()"));

	TPWeaponMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	InteractCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TPWeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TPWeaponMesh->SetSimulatePhysics(true);

	SetInstigator(nullptr);
	SetOwner(nullptr);
}

void AFPSWeaponBase::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (GetOwner() != nullptr)	// OnEquip
	{
		TPWeaponMesh->SetSimulatePhysics(false);
		TPWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		InteractCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else // OnDrop
	{
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
void AFPSWeaponBase::Server_OnBeginFireWeapon_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Server_OnBeginFireWeapon_Implementation"));
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
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Multicast_FireEffects_Implementation"));
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	bool bIsLocalPlayer = UGameplayStatics::GetPlayerPawn(World, 0) == GetOwner();
	if (bIsLocalPlayer == false)
	{
		PlayFireEmitter(false);
		PlayFireSound(false);
	}
}

void AFPSWeaponBase::Server_OnEndFireWeapon_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Server_OnEndFireWeapon_Implementation"));
}
#pragma endregion

#pragma region Client Fire
void AFPSWeaponBase::Client_OnBeginFireWeapon_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Client_OnBeginFireWeapon_Implementation"));
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
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Client_FireEffects_Implementation"));
	PlayFireEmitter(true);
	PlayFireSound(true);
}

void AFPSWeaponBase::Client_OnEndFireWeapon_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Client_OnEndFireWeapon_Implementation"));
}
#pragma endregion

bool AFPSWeaponBase::CanFire()
{	
	return GetOwner() != nullptr && WeaponInfo.bCanFire;
}

void AFPSWeaponBase::Client_Reload_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Client_Reload_Implementation()"));

	AActor* OwnerCharacter = GetOwner();
	if (OwnerCharacter != nullptr && UKismetSystemLibrary::DoesImplementInterface(OwnerCharacter, UFPSCharacterInterface::StaticClass()))
	{
		USkeletalMeshComponent* ArmMesh = IFPSCharacterInterface::Execute_GetArmMesh(OwnerCharacter);
		if (ArmMesh != nullptr)
		{
			UAnimInstance* AnimInstance = ArmMesh->GetAnimInstance();
			if (AnimInstance != nullptr)
			{
				if (WeaponInfo.FP_ArmsReloadAnim != nullptr)
				{
					AnimInstance->Montage_Play(WeaponInfo.FP_ArmsReloadAnim);
				}

				if (WeaponInfo.FP_WeaponReloadAnim != nullptr)
				{
					FPWeaponMesh->PlayAnimation(WeaponInfo.FP_WeaponReloadAnim, false);
				}
			}
		}
	}
}

void AFPSWeaponBase::Server_Reload_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Server_Reload_Implementation"));
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
				UGameplayStatics::SpawnEmitterAttached(WeaponInfo.FireEmitter, FPWeaponMesh, WeaponInfo.FP_FireEmitterSocketName);
			}
			else
			{
				UGameplayStatics::SpawnEmitterAttached(WeaponInfo.FireEmitter, TPWeaponMesh, WeaponInfo.TP_FireEmitterSocketName);
			}
		}
	}
}

void AFPSWeaponBase::PlayFireSound(bool FPWeapon)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::PlayFireSound"));
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

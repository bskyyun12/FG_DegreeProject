// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/FPSWeaponBase.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h" // GetLifetimeReplicatedProps

#include "./FPSCharacterInterface.h"
#include "FPSCharacter.h"

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

void AFPSWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSWeaponBase, OwnerCharacter);
}

void AFPSWeaponBase::Client_OnFPWeaponEquipped_Implementation(AFPSCharacter* FPSCharacter)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Client_OnFPWeaponEquipped_Implementation()"));
}

void AFPSWeaponBase::Client_OnFPWeaponDroped_Implementation(AFPSCharacter* FPSCharacter)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Client_OnFPWeaponDroped_Implementation()"));

}

void AFPSWeaponBase::Server_OnTPWeaponEquipped_Implementation(AFPSCharacter* FPSCharacter)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Server_OnTPWeaponEquipped_Implementation()"));

	if (!ensure(FPSCharacter != nullptr))
	{
		return;
	}

	SetOwner(FPSCharacter);
	SetInstigator(FPSCharacter);
	OwnerCharacter = FPSCharacter; // OnRep_OwnerChanged()

	InteractCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFPSWeaponBase::Server_OnTPWeaponDroped_Implementation(AFPSCharacter* FPSCharacter)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Server_OnTPWeaponDroped_Implementation()"));
	SetOwner(nullptr);
	SetInstigator(nullptr);
	OwnerCharacter = nullptr; // OnRep_OwnerChanged()
}

void AFPSWeaponBase::OnRep_OwnerChanged()
{
	TPWeaponMesh->SetSimulatePhysics(false);
	TPWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InteractCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TPWeaponMesh->AttachToComponent(OwnerCharacter->GetCharacterMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Weapon_Rifle"));
}

bool AFPSWeaponBase::CanFire()
{
	// TODO: implement CanFire
	return true;
}

AFPSWeaponBase* AFPSWeaponBase::GetWeapon_Implementation()
{
	return this;
}

void AFPSWeaponBase::Server_OnBeginFireWeapon_Implementation(AFPSCharacter* FPSCharacter)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Server_OnBeginFireWeapon_Implementation"));
}

void AFPSWeaponBase::Server_OnEndFireWeapon_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Server_OnEndFireWeapon_Implementation"));
}

void AFPSWeaponBase::Client_OnBeginFireWeapon_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Client_OnBeginFireWeapon_Implementation"));
}

void AFPSWeaponBase::Client_OnEndFireWeapon_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Client_OnEndFireWeapon_Implementation"));
}

EWeaponType AFPSWeaponBase::GetWeaponType()
{
	return WeaponType;
}
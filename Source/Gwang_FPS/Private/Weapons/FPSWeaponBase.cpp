// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/FPSWeaponBase.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h" // DoesImplementInterface
#include "Net/UnrealNetwork.h" // GetLifetimeReplicatedProps

#include "./FPSCharacterInterface.h"
#include "Components/HealthComponent.h"
#include "FPSCharacter.h"

// Sets default values
AFPSWeaponBase::AFPSWeaponBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	this->SetRootComponent(RootComp);

	ClientWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ClientWeapnMesh"));
	ClientWeaponMesh->SetOnlyOwnerSee(true);
	ClientWeaponMesh->SetupAttachment(RootComp);

	RepWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RepWeaponMesh->SetOwnerNoSee(true);
	RepWeaponMesh->SetSimulatePhysics(true);
	RepWeaponMesh->SetCollisionProfileName("IgnoreCharacter");
	RepWeaponMesh->SetupAttachment(RootComp);

	InteractCollider = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	InteractCollider->SetSphereRadius(120.f);
	InteractCollider->SetupAttachment(RepWeaponMesh);
	InteractCollider->OnComponentBeginOverlap.AddDynamic(this, &AFPSWeaponBase::OnBeginOverlap);
	InteractCollider->OnComponentEndOverlap.AddDynamic(this, &AFPSWeaponBase::OnEndOverlap);

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
	//DOREPLIFETIME(AFPSWeaponBase, OwnerCharacterMesh);
	DOREPLIFETIME(AFPSWeaponBase, OwnerCharacter);
}

void AFPSWeaponBase::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
	{
		if (UKismetSystemLibrary::DoesImplementInterface(OtherActor, UFPSCharacterInterface::StaticClass()))
		{
			IFPSCharacterInterface::Execute_OnBeginOverlapWeapon(OtherActor, this);
		}
	}
}

void AFPSWeaponBase::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (HasAuthority())
	{
		if (UKismetSystemLibrary::DoesImplementInterface(OtherActor, UFPSCharacterInterface::StaticClass()))
		{
			IFPSCharacterInterface::Execute_OnEndOverlapWeapon(OtherActor);
		}
	}
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
	OwnerCharacter = FPSCharacter; // OnRep_OwnerChanged()

	InteractCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFPSWeaponBase::Server_OnTPWeaponDroped_Implementation(AFPSCharacter* FPSCharacter)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Server_OnTPWeaponDroped_Implementation()"));
	SetOwner(nullptr);
	OwnerCharacter = nullptr; // OnRep_OwnerChanged()
}

void AFPSWeaponBase::OnRep_OwnerChanged()
{
	RepWeaponMesh->SetSimulatePhysics(false);
	RepWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InteractCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RepWeaponMesh->AttachToComponent(OwnerCharacter->GetCharacterMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Weapon_Rifle"));
}

void AFPSWeaponBase::Server_OnBeginFireWeapon_Implementation(AFPSCharacter* FPSCharacter)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Server_OnBeginFireWeapon_Implementation"));
}

void AFPSWeaponBase::Server_OnEndFireWeapon_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Server_OnEndFireWeapon_Implementation"));

}

EWeaponType AFPSWeaponBase::GetWeaponType()
{
	return WeaponType;
}
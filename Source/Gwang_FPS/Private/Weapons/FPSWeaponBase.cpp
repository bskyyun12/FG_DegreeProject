// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/FPSWeaponBase.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h" // DoesImplementInterface
#include "Net/UnrealNetwork.h" // GetLifetimeReplicatedProps
#include "PhysicalMaterials/PhysicalMaterial.h"

#include "./FPSCharacterInterface.h"
#include "AnimInstances/FPSAnimInterface.h"
#include "Components/HealthComponent.h"

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
	DOREPLIFETIME(AFPSWeaponBase, OwnerCharacterMesh);
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

void AFPSWeaponBase::Client_OnClientWeaponEquipped_Implementation(USkeletalMeshComponent* FPSArmMesh)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Client_OnClientWeaponEquipped_Implementation()"));
	ClientWeaponMesh->AttachToComponent(FPSArmMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Weapon_Rifle"));

	UAnimInstance* FPSArmsAnim = FPSArmMesh->GetAnimInstance();
	if (!ensure(FPSArmsAnim != nullptr))
	{
		return;
	}
	if (UKismetSystemLibrary::DoesImplementInterface(FPSArmsAnim, UFPSAnimInterface::StaticClass()))
	{
		IFPSAnimInterface::Execute_UpdateBlendPose(FPSArmsAnim, 1);
	}
}

void AFPSWeaponBase::Client_OnClientWeaponDroped_Implementation(USkeletalMeshComponent* FPSArmMesh)
{

}

void AFPSWeaponBase::Server_OnRepWeaponEquipped_Implementation(USkeletalMeshComponent* FPSCharacterMesh)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Server_OnRepWeaponEquipped_Implementation()"));

	if (!ensure(FPSCharacterMesh != nullptr))
	{
		return;
	}
	OwnerCharacterMesh = FPSCharacterMesh; // OnRep_OwnerChanged()

	RepWeaponMesh->SetSimulatePhysics(false);
	RepWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InteractCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RepWeaponMesh->AttachToComponent(OwnerCharacterMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Weapon_Rifle"));
}

void AFPSWeaponBase::Server_OnRepWeaponDroped_Implementation(USkeletalMeshComponent* FPSCharacterMesh)
{
	OwnerCharacterMesh = nullptr; // OnRep_OwnerChanged()
}

void AFPSWeaponBase::OnRep_OwnerChanged()
{
	RepWeaponMesh->SetSimulatePhysics(false);
	RepWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InteractCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RepWeaponMesh->AttachToComponent(OwnerCharacterMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Weapon_Rifle"));
}

void AFPSWeaponBase::Server_FireWeapon_Implementation(FTransform CameraTransform)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSWeaponBase::Server_FireWeapon_Implementation"));
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	FHitResult Hit;
	FVector Start = CameraTransform.GetLocation();
	FVector End = Start + CameraTransform.GetRotation().GetForwardVector() * WeaponInfo.Range;
	DrawDebugLine(World, Start, End, FColor::Red, true);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());
	Params.bReturnPhysicalMaterial = true;
	bool bIsHit = World->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1, Params); // ECC_GameTraceChannel1 = DamageTrace
	if (bIsHit)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor != nullptr)
		{
			EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
			float DamageMultiplier = 1.f;
			switch (SurfaceType)
			{
			case SurfaceType_Default:
				DamageMultiplier = 1.1f;
				break;
			case SurfaceType1:	// Head
				DamageMultiplier = 2.f;
				break;
			case SurfaceType2:	// Torso
				DamageMultiplier = 1.5f;
				break;
			case SurfaceType3:	// Arms
				DamageMultiplier = 1.2f;
				break;
			case SurfaceType4:	// Legs
				DamageMultiplier = 1.3f;
				break;
			case SurfaceType5:	// Pelvis
				DamageMultiplier = 1.4f;
				break;
			}
			float FinalDamage = WeaponInfo.Damage * DamageMultiplier;

			UHealthComponent* HealthComp = Cast<UHealthComponent>(HitActor->GetComponentByClass(UHealthComponent::StaticClass()));
			if (HealthComp != nullptr && !HealthComp->IsDead())
			{
				HealthComp->AddHealth(-FinalDamage);
			}
			UE_LOG(LogTemp, Warning, TEXT("Damage Taken: %f, Attacker: %s, Damaged Actor: %s"), FinalDamage, *GetOwner()->GetName(), *HitActor->GetName());
		}

		DrawDebugPoint(World, Hit.ImpactPoint, 10.f, FColor::Green, true);
	}
}

EWeaponType AFPSWeaponBase::GetWeaponType()
{
	return WeaponType;
}
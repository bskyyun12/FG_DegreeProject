// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/GrenadeBase.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

#include "DeathMatchCharacter.h"
#include "DeathMatchPlayerController.h"

// Temp
#include "DrawDebugHelpers.h"

// Temp
FColor AGrenadeBase::GetRoleColor()
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

AGrenadeBase::AGrenadeBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

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

	// Create a collider for players to interact with
	InteractCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractCollider"));
	InteractCollider->SetCollisionProfileName(TEXT("NoCollision"));
	InteractCollider->SetupAttachment(TPWeaponMesh);

	// Replicate frequency
	NetUpdateFrequency = 1.f;
}

void AGrenadeBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGrenadeBase, ServerExplosionTime);
}

void AGrenadeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SimulateTrajectory(DeltaSeconds, true);

	if (GetLocalRole() == ROLE_Authority)
	{
		ServerExplosionTime += DeltaSeconds;
		if (ServerExplosionTime > WeaponInfo.ExplodeInSeconds)
		{
			Explode();
			SetActorTickEnabled(false);
		}
	}
	else
	{
		ClientExplosionTime += DeltaSeconds;
		if (ClientExplosionTime > WeaponInfo.ExplodeInSeconds)
		{
			Explode();
			SetActorTickEnabled(false);
		}
	}
}

void AGrenadeBase::Explode()
{
	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		DrawDebugSphere(World, NewLocation, WeaponInfo.ExplosionRadius, 15, FColor::Red, false, 5.f);
	}
}

#pragma region Getters & Setters
ADeathMatchCharacter* AGrenadeBase::GetCurrentOwner()
{
	if (GetOwner() == nullptr)
	{
		return nullptr;
	}

	return Cast<ADeathMatchCharacter>(GetOwner());
}

ADeathMatchPlayerController* AGrenadeBase::GetOwnerController()
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

EWeaponType AGrenadeBase::GetWeaponType_Implementation() const
{
	return WeaponInfo.WeaponType;
}

void AGrenadeBase::SetVisibility_Implementation(bool NewVisibility)
{
	FPWeaponMesh->SetVisibility(NewVisibility);
	TPWeaponMesh->SetVisibility(NewVisibility);
}
#pragma endregion Getters & Setters

#pragma region Weapon Equip & Drop
void AGrenadeBase::OnWeaponEquipped_Implementation(ADeathMatchCharacter* NewOwner)
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
		UpdateAmmoUI(1, 0);
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

void AGrenadeBase::OnWeaponDropped_Implementation()
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

void AGrenadeBase::UpdateAmmoUI(const int& InCurrentAmmo, const int& InRemainingAmmo)
{
	if (GetCurrentOwner()->IsLocallyControlled())
	{
		if (GetOwnerController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetOwnerController(), UPlayerControllerInterface::StaticClass()))
		{
			IPlayerControllerInterface::Execute_UpdateWeaponUI(GetOwnerController(), WeaponInfo.DisplayName, InCurrentAmmo, InRemainingAmmo);
		}
	}
}
#pragma endregion Weapon Equip & Drop

#pragma region EndFire (Throw Grenade)
void AGrenadeBase::EndFire_Implementation()
{
	if (GetCurrentOwner() != nullptr)
	{
		if (GetCurrentOwner()->IsLocallyControlled())
		{
			Fire();
		}

		if (GetCurrentOwner()->GetLocalRole() == ROLE_Authority)
		{
			Multicast_Fire();
		}

		if (GetCurrentOwner()->GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_Fire();
		}
	}
}

void AGrenadeBase::Server_Fire_Implementation()
{
	Multicast_Fire();
}

void AGrenadeBase::Multicast_Fire_Implementation()
{
	if (GetCurrentOwner() != nullptr && !GetCurrentOwner()->IsLocallyControlled())
	{
		Fire();
	}
}

void AGrenadeBase::Fire()
{
	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		World->GetTimerManager().ClearTimer(PathDrawingtimer);
	}

	InitTrajectory();

	if (GetLocalRole() == ROLE_Authority)
	{
		ServerExplosionTime = 0.f;
	}
	else
	{
		ClientExplosionTime = 0.f;
	}

	SetActorTickEnabled(true);
}

void AGrenadeBase::OnRep_ServerExplosionTime()
{
	float NetworkLatency = ClientExplosionTime - ServerExplosionTime;
	ClientExplosionTime -= NetworkLatency;
	//FlightTime += NetworkLatency;

	UE_LOG(LogTemp, Warning, TEXT("AGrenadeBase::OnRep_ServerExplosionTime NetworkLatency: ( %f )"), NetworkLatency);
}
#pragma endregion EndFire (Throw Grenade)

#pragma region BeginFire (Grenade Path, Local only)
void AGrenadeBase::BeginFire_Implementation()
{
	if (GetCurrentOwner() != nullptr && GetCurrentOwner()->IsLocallyControlled())
	{
		UWorld* World = GetWorld();
		if (World != nullptr)
		{
			World->GetTimerManager().SetTimer(PathDrawingtimer, this, &AGrenadeBase::DrawGrenadePath, PathDrawingFrequency, true);
		}
	}
}

void AGrenadeBase::DrawGrenadePath()
{
	InitTrajectory();

	for (float Time = 0.f; Time < 2.f; Time += PathDrawingFrequency)
	{
		SimulateTrajectory(PathDrawingFrequency, false);
	}
}
#pragma endregion BeginFire (Grenade Path, Local only)

#pragma region Trajectory
void AGrenadeBase::InitTrajectory()
{
	if (GetCurrentOwner() != nullptr)
	{
		Trajectory.LaunchLocation = FPWeaponMesh->GetComponentLocation();
		Trajectory.LaunchForward = GetOwner()->GetActorForwardVector();
		Trajectory.LaunchAngleInRad = GetCurrentOwner()->GetCameraPitch() * PI / 180.f;
		Trajectory.LaunchSpeed = WeaponInfo.ThrowingPower;
		PrevLocation = Trajectory.LaunchLocation;
		FlightTime = 0.f;
	}
}

void AGrenadeBase::SimulateTrajectory(const float& DeltaSeconds, bool bMoveMesh)
{
	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		FlightTime += DeltaSeconds;

		DisplacementX = Trajectory.LaunchSpeed * FlightTime * FMath::Cos(Trajectory.LaunchAngleInRad);
		DisplacementZ = Trajectory.LaunchSpeed * FlightTime * FMath::Sin(Trajectory.LaunchAngleInRad) - 0.5f * Gravity * FlightTime * FlightTime;
		NewLocation = Trajectory.LaunchLocation + Trajectory.LaunchForward * DisplacementX + FVector::UpVector * DisplacementZ;

		if (bMoveMesh)
		{
			// TODO: Move Mesh here
			DrawDebugLine(World, PrevLocation, NewLocation, GetRoleColor(), false, DeltaSeconds * 10.f, 0, 7.f);
		}
		else
		{
			DrawDebugLine(World, PrevLocation, NewLocation, GetRoleColor(), false, DeltaSeconds, 0, 7.f);
		}

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		Params.AddIgnoredActor(this->GetOwner());
		if (World->LineTraceSingleByChannel(Hit, PrevLocation, NewLocation, ECC_Visibility, Params))
		{
			if (bMoveMesh)
			{
				DrawDebugPoint(World, Hit.ImpactPoint, 20.f, FColor::Red, false, DeltaSeconds * 10.f);
			}
			else
			{
				DrawDebugPoint(World, Hit.ImpactPoint, 20.f, FColor::Red, false, DeltaSeconds);
			}

			FVector Reflection = FMath::GetReflectionVector((Hit.ImpactPoint - PrevLocation).GetSafeNormal(), Hit.ImpactNormal);
			FVector RightVector = FVector::CrossProduct(Reflection, FVector::UpVector);
			Trajectory.LaunchForward = FVector::CrossProduct(FVector::UpVector, RightVector);

			// arccos dot A, B returns angle between A, B
			Trajectory.LaunchAngleInRad = FMath::Acos(FVector::DotProduct(Reflection, Trajectory.LaunchForward));
			// If the reflection vector heads downwards, we need the negative angle
			Trajectory.LaunchAngleInRad *= FMath::Sign(Reflection.Z);

			FlightTime = 0.f;

			Trajectory.LaunchSpeed *= WeaponInfo.Bounceness;

			Trajectory.LaunchLocation = Hit.ImpactPoint + Hit.ImpactNormal;
			NewLocation = Hit.ImpactPoint + Hit.ImpactNormal;
		}

		PrevLocation = NewLocation;
	}
}
#pragma endregion Trajectory

// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/GrenadeBase.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
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
	if (LatestOwner != nullptr)
	{
		if (LatestOwner->GetLocalRole() == ROLE_Authority)
		{
			return FColor::Red;
		}

		if (LatestOwner->GetLocalRole() == ROLE_AutonomousProxy)
		{
			return FColor::Green;
		}

		if (LatestOwner->GetLocalRole() == ROLE_SimulatedProxy)
		{
			return FColor::Blue;
		}
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

	// Create a collider for explosion
	ExplosionCollider = CreateDefaultSubobject<USphereComponent>(TEXT("ExplosionCollider"));
	ExplosionCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ExplosionCollider->InitSphereRadius(GrenadeInfo.ExplosionRadius);
	ExplosionCollider->SetupAttachment(RootComponent);

	// Replicate frequency
	NetUpdateFrequency = 1.f;
}

void AGrenadeBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AGrenadeBase, ServerExplosionTime, COND_InitialOnly);
	DOREPLIFETIME(AGrenadeBase, LatestOwner);
}

void AGrenadeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SimulateTrajectory(DeltaSeconds, true);

	if (GetLocalRole() == ROLE_Authority)
	{
		ServerExplosionTime += DeltaSeconds;
		if (ServerExplosionTime > GrenadeInfo.ExplodeInSeconds)
		{
			Explode();
		}
	}
	else
	{
		ClientExplosionTime += DeltaSeconds;
		if (ClientExplosionTime > GrenadeInfo.ExplodeInSeconds)
		{
			Explode();
		}
	}
}

void AGrenadeBase::Explode()
{
	ExplosionEffects();

	if (GetLocalRole() == ROLE_Authority)
	{
		ExplosionCollider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		TArray<AActor*> ActorsWithinRange;
		ExplosionCollider->GetOverlappingActors(ActorsWithinRange);

		for (AActor* Actor : ActorsWithinRange)
		{
			UE_LOG(LogTemp, Warning, TEXT("GunBase::Explode => OverlappingActor ( %s )"), *Actor->GetName());

			ADeathMatchCharacter* Player = Cast<ADeathMatchCharacter>(Actor);
			if (Player != nullptr)
			{
				float DamageOnHealth = GrenadeInfo.ExplosionDamage * GrenadeInfo.ArmorPenetration;
				float DamageOnArmor = GrenadeInfo.ExplosionDamage * (1 - GrenadeInfo.ArmorPenetration);
				Player->Server_TakeDamage(DamageOnHealth, DamageOnArmor, LatestOwner);
			}
		}

		ExplosionCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	FPWeaponMesh->SetVisibility(false, true);
	TPWeaponMesh->SetVisibility(false, true);
	SetActorTickEnabled(false);
}

void AGrenadeBase::ExplosionEffects()
{
	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		DrawDebugSphere(World, NewLocation, GrenadeInfo.ExplosionRadius, 10, GetRoleColor(), false, 5.f, 0, 5.f);

		// Sound
		if (GrenadeInfo.ExplosionSound != nullptr)
		{
			UGameplayStatics::SpawnSoundAtLocation(World, GrenadeInfo.ExplosionSound, NewLocation);
		}

		// Emitter
		if (GrenadeInfo.ExplosionEmitter != nullptr)
		{
			UGameplayStatics::SpawnEmitterAtLocation(World, GrenadeInfo.ExplosionEmitter, NewLocation);
		}
	}
}

#pragma region Getters & Setters
EWeaponType AGrenadeBase::GetWeaponType_Implementation() const
{
	return GrenadeInfo.WeaponType;
}

void AGrenadeBase::SetVisibility_Implementation(bool NewVisibility)
{
	FPWeaponMesh->SetVisibility(NewVisibility);
	TPWeaponMesh->SetVisibility(NewVisibility);
}
#pragma endregion Getters & Setters

#pragma region Weapon Equip
void AGrenadeBase::OnWeaponEquipped_Implementation(ADeathMatchCharacter* NewOwner)
{
	UE_LOG(LogTemp, Warning, TEXT("GunBase::OnWeaponEquipped => NewOwner(%s)'s role: %i. / Weapon(%s)'s role: %i."), *NewOwner->GetName(), NewOwner->GetLocalRole(), *GetName(), GetLocalRole());
	if (!ensure(NewOwner != nullptr))
	{
		return;
	}
	LatestOwner = NewOwner;
	SetOwner(NewOwner);

	// skeleton is not yet created in the constructor, so AttachToComponent should be happened after constructor
	TPWeaponMesh->SetSimulatePhysics(false);
	TPWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TPWeaponMesh->AttachToComponent(NewOwner->GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), GrenadeInfo.TP_SocketName);
	FPWeaponMesh->AttachToComponent(NewOwner->GetArmMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), GrenadeInfo.FP_SocketName);

	if (NewOwner->IsLocallyControlled())
	{
		// Update Weapon UI
		if (NewOwner->GetController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(NewOwner->GetController(), UPlayerControllerInterface::StaticClass()))
		{
			IPlayerControllerInterface::Execute_UpdateWeaponUI(NewOwner->GetController(), GrenadeInfo.DisplayName, 1, 0);
		}

		// Play FP_EquipAnim
		UAnimInstance* AnimInstance = NewOwner->GetArmMesh()->GetAnimInstance();
		if (AnimInstance != nullptr && GrenadeInfo.FP_EquipAnim != nullptr)
		{
			AnimInstance->Montage_Play(GrenadeInfo.FP_EquipAnim);
		}
	}
	else
	{
		// TODO: Play TP_EquipAnim
	}
}
#pragma endregion Weapon Equip

#pragma region EndFire (Throw Grenade)
void AGrenadeBase::EndFire_Implementation()
{
	if (bIsUsed)
	{
		return;
	}

	if (LatestOwner != nullptr)
	{
		if (LatestOwner->IsLocallyControlled())
		{
			Fire();

			// Update Weapon UI
			if (LatestOwner->GetController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(LatestOwner->GetController(), UPlayerControllerInterface::StaticClass()))
			{
				IPlayerControllerInterface::Execute_UpdateWeaponUI(LatestOwner->GetController(), GrenadeInfo.DisplayName, 0, 0);
			}
		}

		if (LatestOwner->GetLocalRole() == ROLE_Authority)
		{
			Multicast_Fire();
		}

		if (LatestOwner->GetLocalRole() == ROLE_AutonomousProxy)
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
	if (LatestOwner != nullptr && !LatestOwner->IsLocallyControlled())
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

	bIsUsed = true;

	InitTrajectory();

	if (GetLocalRole() == ROLE_Authority)
	{
		ServerExplosionTime = 0.01f;
	}
	else
	{
		ClientExplosionTime = 0.f;
	}

	FPWeaponMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	TPWeaponMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	SetActorTickEnabled(true);
}

void AGrenadeBase::OnRep_ServerExplosionTime()
{
	UE_LOG(LogTemp, Warning, TEXT("AGrenadeBase::OnRep_ServerExplosionTime ClientExplosionTime: ( %f ), ServerExplosionTime: ( %f )"), ClientExplosionTime, ServerExplosionTime);
	//float NetworkLatency = ClientExplosionTime - ServerExplosionTime;
	//ClientExplosionTime -= NetworkLatency;
	//FlightTime += NetworkLatency;

	ClientExplosionTime = ServerExplosionTime;
}
#pragma endregion EndFire (Throw Grenade)

#pragma region BeginFire (Grenade Path, Local only)
void AGrenadeBase::BeginFire_Implementation()
{
	if (bIsUsed)
	{
		return;
	}

	if (LatestOwner != nullptr && LatestOwner->IsLocallyControlled())
	{
		UWorld* World = GetWorld();
		if (World != nullptr)
		{
			World->GetTimerManager().SetTimer(PathDrawingtimer, this, &AGrenadeBase::DrawGrenadePath, World->GetDeltaSeconds(), true);
		}
	}
}

void AGrenadeBase::DrawGrenadePath()
{
	InitTrajectory();
	
	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		for (float Time = 0.f; Time < 1.f; Time += World->GetDeltaSeconds())
		{
			SimulateTrajectory(World->GetDeltaSeconds(), false);
		}
	}
}
#pragma endregion BeginFire (Grenade Path, Local only)

#pragma region Trajectory
void AGrenadeBase::InitTrajectory()
{
	if (LatestOwner != nullptr)
	{
		Trajectory.LaunchLocation = FPWeaponMesh->GetComponentLocation();
		Trajectory.LaunchForward = LatestOwner->GetActorForwardVector();
		Trajectory.LaunchAngleInRad = LatestOwner->GetCameraPitch() * PI / 180.f;
		Trajectory.LaunchSpeed = GrenadeInfo.ThrowingPower;
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
			DrawDebugLine(World, PrevLocation, NewLocation, GetRoleColor(), false, DeltaSeconds * 10.f, 0, 7.f);
			FPWeaponMesh->SetWorldLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
			TPWeaponMesh->SetWorldLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
			ExplosionCollider->SetWorldLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
		}
		else
		{
			DrawDebugLine(World, PrevLocation, NewLocation, GetRoleColor(), false, DeltaSeconds, 0, 7.f);
		}

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		Params.AddIgnoredActor(LatestOwner);
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

			Trajectory.LaunchSpeed *= GrenadeInfo.Bounceness;

			Trajectory.LaunchLocation = Hit.ImpactPoint + Hit.ImpactNormal;
			NewLocation = Hit.ImpactPoint + Hit.ImpactNormal;
		}

		PrevLocation = NewLocation;
	}
}
#pragma endregion Trajectory

// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/FPSGrenadeBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

// Temp
#include "DrawDebugHelpers.h"

void AFPSGrenadeBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSGrenadeBase, ServerMove);
	DOREPLIFETIME(AFPSGrenadeBase, bSimulateGrenadeMove);
}

AFPSGrenadeBase::AFPSGrenadeBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFPSGrenadeBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		NetUpdateFrequency = 2.f;
	}

	World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
}

void AFPSGrenadeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetOwner() == nullptr)
	{
		return;
	}

	bIsLocallyControlled = Cast<APawn>(GetOwner())->IsLocallyControlled();
	bool IsHostPlayer = GetOwner()->GetLocalRole() == ROLE_Authority && bIsLocallyControlled;

	if (bIsLocallyControlled && bDrawtrajectory)
	{
		DrawGrenadePath();
	}

	if (bSimulateGrenadeMove)
	{
		// Locally controlled host or locally controlled client
		if (IsHostPlayer || GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
		{
			// Simulate Movement
			MoveGrenade(ClientMove);

			// Update server's move state, simulate trajectory if it's not host player. (Since the host player simulate the trajectory already in 2nd step)
			Server_UpdateMoveState(ClientMove, !IsHostPlayer);
		}

		// Other clients
		if (GetOwner()->GetLocalRole() == ROLE_SimulatedProxy)
		{
			// Get move from ServerState (ServerState is replicated)
			MoveGrenade(ServerMove);
		}
	}
	else
	{
		if (ArmMesh != nullptr && CharacterMesh != nullptr)
		{
			//FPWeaponMesh->SetWorldLocation(ArmMesh->GetSocketLocation(WeaponInfo.FP_ArmsSocketName));
			//TPWeaponMesh->SetWorldLocation(CharacterMesh->GetSocketLocation(WeaponInfo.TP_CharacterSocketName));
		}
	}
}

//void AFPSGrenadeBase::Server_OnWeaponEquipped_Implementation(AFPSCharacter* OwnerCharacter)
//{
//	Super::Server_OnWeaponEquipped_Implementation(OwnerCharacter);
//
//	if (UKismetSystemLibrary::DoesImplementInterface(GetOwner(), UFPSCharacterInterface::StaticClass()))
//	{
//		ArmMesh = IFPSCharacterInterface::Execute_GetArmMesh(GetOwner());
//		CharacterMesh = IFPSCharacterInterface::Execute_GetCharacterMesh(GetOwner());
//	}
//}
//
//void AFPSGrenadeBase::OnRep_Owner()
//{
//	Super::OnRep_Owner();
//
//	if (UKismetSystemLibrary::DoesImplementInterface(GetOwner(), UFPSCharacterInterface::StaticClass()))
//	{
//		ArmMesh = IFPSCharacterInterface::Execute_GetArmMesh(GetOwner());
//		CharacterMesh = IFPSCharacterInterface::Execute_GetCharacterMesh(GetOwner());
//	}
//}

FGrenadeMove AFPSGrenadeBase::InitializeTrajectory(float DeltaSeconds)
{
	FGrenadeMove Move;
	//Move.LaunchPoint = FPWeaponMesh->GetComponentLocation() + GetOwner()->GetActorForwardVector();
	//Move.PrevPoint = Move.LaunchPoint;
	//Move.LaunchForward = GetOwner()->GetActorForwardVector();
	//Move.CurrentSpeed = LaunchSpeed;
	//Move.FlightTime = 0.f;
	//Move.LifeTime = 0.f;
	//Move.DeltaSeconds = DeltaSeconds;
	//Move.LaunchAngleInRad = GetOwner()->GetInstigatorController()->GetControlRotation().Pitch * PI / 180.f;

	return Move;
}

void AFPSGrenadeBase::Server_UpdateMoveState_Implementation(FGrenadeMove Move, bool bShouldServerSimulateMove)
{
	if (bShouldServerSimulateMove)
	{
		MoveGrenade(Move);
	}

	ServerMove = Move;	// OnRep_ServerMove()
}

void AFPSGrenadeBase::OnRep_ServerMove()
{
	if (GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
	{

	}

	if (GetOwner()->GetLocalRole() == ROLE_SimulatedProxy)
	{

	}
}

void AFPSGrenadeBase::DrawGrenadePath()
{
	FGrenadeMove Move = InitializeTrajectory(TrajectoryDrawingDeltaSeconds);
	for (float Time = 0.f; Time < 1.f; Time += TrajectoryDrawingDeltaSeconds)
	{
		CalcTrajectory(Move);
	}
}

void AFPSGrenadeBase::MoveGrenade(FGrenadeMove& Move)
{
	Move.LifeTime += World->GetDeltaSeconds();

	if (Move.LifeTime > ExplodeDelay)
	{
		// Todo: Here implement explosion
		DrawDebugSphere(World, Move.NewPoint, ExplodeRadius, 10, FColor::Red, false, 3.f);
		bSimulateGrenadeMove = false;
	}

	//if (bIsLocallyControlled)
	//{
	//	if (FPWeaponMesh != nullptr)
	//	{
	//		FPWeaponMesh->SetWorldLocation(Move.NewPoint);
	//	}
	//}
	//else
	//{
	//	if (TPWeaponMesh != nullptr)
	//	{
	//		TPWeaponMesh->SetWorldLocation(Move.NewPoint);
	//	}
	//}

	DrawDebugPoint(World, Move.NewPoint, 10.f, FColor::Purple, false, .5f);
	CalcTrajectory(Move);
}

void AFPSGrenadeBase::CalcTrajectory(FGrenadeMove& Move)
{
	Move.FlightTime += Move.DeltaSeconds;

	float DisplacementX = Move.CurrentSpeed * Move.FlightTime * FMath::Cos(Move.LaunchAngleInRad);
	float DisplacementZ = Move.CurrentSpeed * Move.FlightTime * FMath::Sin(Move.LaunchAngleInRad) - 0.5f * GRAVITY * Move.FlightTime * Move.FlightTime;
	Move.NewPoint = Move.LaunchPoint + Move.LaunchForward * DisplacementX + FVector::UpVector * DisplacementZ;

	DrawDebugLine(World, Move.PrevPoint, Move.NewPoint, FColor::Green, false, -1.f, 0, 7.f);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(this->GetOwner());
	if (World->LineTraceSingleByChannel(Hit, Move.PrevPoint, Move.NewPoint, ECC_Visibility, Params))
	{
		DrawDebugPoint(World, Hit.ImpactPoint, 10.f, FColor::Red);
		DrawDebugPoint(World, Move.PrevPoint, 10.f, FColor::Black);

		FVector Reflection = FMath::GetReflectionVector((Hit.ImpactPoint - Move.PrevPoint).GetSafeNormal(), Hit.ImpactNormal);
		FVector RightVector = FVector::CrossProduct(Reflection, FVector::UpVector);
		Move.LaunchForward = FVector::CrossProduct(FVector::UpVector, RightVector);

		Move.LaunchAngleInRad = FMath::Acos(FVector::DotProduct(Reflection, Move.LaunchForward));
		Move.LaunchAngleInRad *= FMath::Sign(Reflection.Z);

		Move.LaunchPoint = Hit.ImpactPoint + Hit.ImpactNormal;
		Move.NewPoint = Move.LaunchPoint;

		Move.FlightTime = 0.f;

		Move.CurrentSpeed *= 0.5f;
	}

	Move.PrevPoint = Move.NewPoint;
}

void AFPSGrenadeBase::Server_OnBeginFireWeapon_Implementation()
{

}

void AFPSGrenadeBase::Server_OnEndFireWeapon_Implementation()
{
	bSimulateGrenadeMove = true;
}

void AFPSGrenadeBase::Client_OnBeginFireWeapon_Implementation()
{
	bDrawtrajectory = true;
}

void AFPSGrenadeBase::Client_OnEndFireWeapon_Implementation()
{
	bDrawtrajectory = false;
	bSimulateGrenadeMove = true;

	if (GetOwner() != nullptr)
	{
		ClientMove = InitializeTrajectory(World->GetDeltaSeconds());
	}
}
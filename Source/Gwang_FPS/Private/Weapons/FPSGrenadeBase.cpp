// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/FPSGrenadeBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

#include "FPSCharacterInterface.h"

// Temp
#include "DrawDebugHelpers.h"

void AFPSGrenadeBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSGrenadeBase, ServerState);
	DOREPLIFETIME(AFPSGrenadeBase, bDrawtrajectory);
}

AFPSGrenadeBase::AFPSGrenadeBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFPSGrenadeBase::BeginPlay()
{
	Super::BeginPlay();

	World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
}

void AFPSGrenadeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bDrawtrajectory)
	{
		bool IsHostPlayer = GetOwner()->GetLocalRole() == ROLE_Authority && Cast<APawn>(GetOwner())->IsLocallyControlled();

		// Locally controlled host or locally controlled client
		if (IsHostPlayer || GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
		{			
			// 1. Created Move
			LastMove = CreateMove();

			// 2. Simulate trajectory
			SimulateTrajectory(LastMove);

			// 3. Update server's move state, simulate trajectory if it's not host player. (Since the host player simulate the trajectory already in 2nd step)
			Server_UpdateMoveState(LastMove, !IsHostPlayer);			
		}

		// Other clients
		if (GetOwner()->GetLocalRole() == ROLE_SimulatedProxy)
		{
			// Get move from ServerState (ServerState is replicated)
			SimulateTrajectory(ServerState.LastMove);
		}
	}
}

FGrenadeMove AFPSGrenadeBase::CreateMove()
{
	FGrenadeMove Move;
	Move.LaunchPoint = FPWeaponMesh->GetComponentLocation() + GetOwner()->GetActorForwardVector();
	Move.PrevPoint = Move.LaunchPoint;
	Move.LaunchForward = GetOwner()->GetActorForwardVector();
	Move.LaunchUp = GetOwner()->GetActorUpVector();
	Move.CurrentSpeed = LaunchSpeed;
	Move.FlightTime = 0.f;
	Move.LaunchAngleInRad = GetOwner()->GetInstigatorController()->GetControlRotation().Pitch * PI / 180.f;
	return Move;
}

void AFPSGrenadeBase::Server_UpdateMoveState_Implementation(FGrenadeMove Move, bool bShouldServerSimulateMove)
{
	// Update ServerState
	ServerState.LastMove = Move;

	if (bShouldServerSimulateMove)
	{
		SimulateTrajectory(Move);
	}
}

void AFPSGrenadeBase::SimulateTrajectory(FGrenadeMove Move)
{
	for (float Time = 0.f; Time < 1.f; Time += PathDrawInSeconds)
	{
		Move.FlightTime += PathDrawInSeconds;

		float DisplacementX = Move.CurrentSpeed * Move.FlightTime * FMath::Cos(Move.LaunchAngleInRad);
		float DisplacementZ = Move.CurrentSpeed * Move.FlightTime * FMath::Sin(Move.LaunchAngleInRad) - 0.5f * GRAVITY * Move.FlightTime * Move.FlightTime;
		Move.NewPoint = Move.LaunchPoint + Move.LaunchForward * DisplacementX + Move.LaunchUp * DisplacementZ;

		DrawDebugLine(World, Move.PrevPoint, Move.NewPoint, FColor::Cyan, false, -1.f, 0, 7.f);

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		Params.AddIgnoredActor(this->GetOwner());
		if (World->LineTraceSingleByChannel(Hit, Move.PrevPoint, Move.NewPoint, ECC_Visibility, Params))
		{
			DrawDebugPoint(World, Hit.ImpactPoint, 30.f, FColor::Orange);

			FVector Reflection = FMath::GetReflectionVector((Hit.ImpactPoint - Move.PrevPoint).GetSafeNormal(), Hit.ImpactNormal);
			FVector RightVector = FVector::CrossProduct(Reflection, Move.LaunchUp);
			Move.LaunchForward = FVector::CrossProduct(Move.LaunchUp, RightVector);

			Move.LaunchAngleInRad = FMath::Acos(FVector::DotProduct(Reflection, Move.LaunchForward));
			Move.LaunchAngleInRad *= FMath::Sign(Reflection.Z);

			Move.LaunchPoint = Hit.ImpactPoint + Hit.ImpactNormal;
			Move.NewPoint = Move.LaunchPoint;

			Move.FlightTime = 0.f;

			Move.CurrentSpeed *= 0.75f;
		}

		Move.PrevPoint = Move.NewPoint;
	}
}

void AFPSGrenadeBase::Server_OnBeginFireWeapon_Implementation()
{
	//Super::Server_OnBeginFireWeapon_Implementation();
	bDrawtrajectory = true;
}

void AFPSGrenadeBase::Server_OnEndFireWeapon_Implementation()
{
	//Super::Server_OnEndFireWeapon_Implementation();
	bDrawtrajectory = false;
}
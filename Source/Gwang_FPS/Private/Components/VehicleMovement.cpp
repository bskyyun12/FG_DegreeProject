// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/VehicleMovement.h"
#include <Net/UnrealNetwork.h>
#include <DrawDebugHelpers.h>

// Sets default values for this component's properties
UVehicleMovement::UVehicleMovement()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}

void UVehicleMovement::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	DrawDebugLine(GetWorld(), ServerState.Transform.GetLocation(), ServerState.Transform.GetLocation() + FVector::UpVector * 200.f, FColor::Green, false, -1.f, 0, 15.f);

	bool HostPlayer = GetOwner()->GetRemoteRole() != ROLE_AutonomousProxy && GetOwner()->GetLocalRole() == ROLE_Authority;
	if (HostPlayer || GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
	{
		ClientMove = CreateMove(DeltaTime);
		SimulateMove(ClientMove);

		if (GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
		{
			ClientMoves.Add(ClientMove);
			Server_SendMove(ClientMove);
		}

		if (HostPlayer)
		{
			UpdateServerState(ClientMove);
		}
	}

	if (GetOwner()->GetLocalRole() == ROLE_SimulatedProxy)
	{
		ClientTick(DeltaTime);
	}
}

FVehicleMove UVehicleMovement::CreateMove(const float& DeltaTime)
{
	FVehicleMove Move;
	Move.Throttle = Throttle;
	Move.SteeringThrow = SteeringThrow;
	Move.bIsDrifting = bIsDrifting;
	Move.DeltaTime = DeltaTime;
	Move.Time = GetWorld()->GetTimeSeconds();

	return Move;
}

void UVehicleMovement::SimulateMove(const FVehicleMove& Move)
{
	FVector Force = GetOwner()->GetActorForwardVector() * MaxDrivingForce * Move.Throttle;
	Force += GetAirResistance();	// Air Resistance = Speed^2 * DragCoefficient
	Force += GetRollingResistance();	// Rolling Resistance = NormalForce(Mass*Gravity) * RollingResistanceCoefficient
	Force += GetSideFriction();	// dot(velocity, normalright) * -normalright * mass

	// Verlet
	FVector NewAcceleration = Force / MassInKilo;
	Velocity += (Acceleration + NewAcceleration) * (Move.DeltaTime * 0.5);
	Acceleration = NewAcceleration;
	//// Euler
	//UpdateLocationFromVelocity(Move.DeltaTime);
	//Acceleration = Force / MassInKilo;
	//Velocity += (Acceleration * Move.DeltaTime);

	ApplyRotation(Move.DeltaTime, Move.SteeringThrow, Move.bIsDrifting);
	UpdateLocationFromVelocity(Move.DeltaTime);
}

void UVehicleMovement::ApplyRotation(const float& DeltaTime, const float& Steering, const bool& Drift)
{
	float DeltaX = FVector::DotProduct(GetOwner()->GetActorForwardVector(), Velocity) * DeltaTime;
	float RotationAngle = DeltaX / (Drift ? MinTurningRadius / DriftPower : MinTurningRadius) * Steering;	// delta angle = dx / radius
	FQuat RotationDelta(GetOwner()->GetActorUpVector(), RotationAngle);

	if (!Drift)
	{
		Velocity = RotationDelta.RotateVector(Velocity);
	}
	else
	{
		CurrentRotationAngle = FMath::LerpStable(CurrentRotationAngle, RotationAngle, SteeringnessOnDrift * DeltaTime);
		FQuat DriftRotationDelta(GetOwner()->GetActorUpVector(), CurrentRotationAngle);
		Velocity = DriftRotationDelta.RotateVector(Velocity);
	}

	GetOwner()->AddActorWorldRotation(RotationDelta, true);
}

void UVehicleMovement::UpdateLocationFromVelocity(const float& DeltaTime)
{
	float M2cm = 100; // Meter to centimeter
	FVector DeltaX;

	// Verlet
	DeltaX = (Velocity * DeltaTime * M2cm) + (Acceleration * DeltaTime * DeltaTime * 0.5);
	//// Euler
	//DeltaX = Velocity * DeltaTime * M2cm;

	FHitResult Hit;
	GetOwner()->AddActorWorldOffset(DeltaX, true, &Hit);
	if (Hit.IsValidBlockingHit())
	{
		UE_LOG(LogTemp, Warning, TEXT("Colliding with %s"), *Hit.GetActor()->GetName());
		HandleCollision(Hit);
	}
}

void UVehicleMovement::HandleCollision(const FHitResult& Hit)
{
	AActor* HitActor = Hit.GetActor();
	if (HitActor == nullptr)
	{
		return;
	}

	FVector V1i = Velocity;
	float M1 = MassInKilo;
	float M2 = MassInKilo;
	FVector V2i = FVector::OneVector;

	UVehicleMovement* MoveComp = HitActor->FindComponentByClass<UVehicleMovement>();
	if (MoveComp != nullptr)
	{
		V2i = MoveComp->GetVelocity();
		M2 = MoveComp->GetMass();
	}

	// this vehicle
	FVector HitDir1 = GetOwner()->GetActorLocation() - Hit.ImpactPoint;
	HitDir1.Z = 0;
	FVector V1f = V1i - ((M2 + M2) / (M1 + M2)) * FVector::DotProduct((V1i - V2i), HitDir1) / (HitDir1.Size() * HitDir1.Size()) * HitDir1;
	Velocity = V1f * Elast;

	// other
	FVector HitDir2 = Hit.ImpactPoint - GetOwner()->GetActorLocation();
	HitDir2.Z = 0;
	if (MoveComp != nullptr)
	{
		FVector V2f = V2i - ((M1 + M1) / (M1 + M2)) * FVector::DotProduct((V2i - V1i), HitDir2) / (HitDir2.Size() * HitDir2.Size()) * HitDir2;
		MoveComp->SetVelocity(V2f * MoveComp->GetElast());
	}
}

FVector UVehicleMovement::GetAirResistance() const
{
	// Air Resistance = Speed^2 * DragCoefficient
	float Speed = Velocity.Size();
	return -Velocity.GetSafeNormal() * Speed * Speed * DragCoefficient;
}

FVector UVehicleMovement::GetRollingResistance() const
{
	// Rolling Resistance = NormalForce(Mass*Gravity) * DragCoefficient
	float Gravity = -GetWorld()->GetGravityZ() / 100;	// 9.81....
	float NormalForce = MassInKilo * Gravity;	// Normal Force = Mass * Gravity
	return -Velocity.GetSafeNormal() * RollingResistanceCoefficient * NormalForce;
}

FVector UVehicleMovement::GetSideFriction() const
{
	// dot(velocity, normalright) * -normalright * mass
	FVector SideFriction = FVector::DotProduct(Velocity, GetOwner()->GetActorRightVector()) * (-1.f * GetOwner()->GetActorRightVector()) * MassInKilo;

	if (SideFriction.Size() > MaxSideFriction * MassInKilo)
	{
		UE_LOG(LogTemp, Warning, TEXT("SideFriction: %f"), SideFriction.Size());
		SideFriction *= MaxSideFriction * MassInKilo / SideFriction.Size();
	}
	return SideFriction;
}

void UVehicleMovement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UVehicleMovement, ServerState);
}

void UVehicleMovement::Server_SendMove_Implementation(const FVehicleMove& Move)
{
	SimulateMove(Move);
	UpdateServerState(Move);
}

void UVehicleMovement::UpdateServerState(const FVehicleMove& Move)
{
	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = Velocity;
}

void UVehicleMovement::OnRep_ServerState()
{
	if (GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
	{
		// I don't want the update too often
		uint16 NumMoves = ClientMoves.Num();
		if (NumMoves < 60)
		{
			return;
		}

		if (VehicleMesh != nullptr)
		{
			// 0. Save Current location before correction
			FVector ClientLocation = VehicleMesh->GetComponentLocation();

			// 1. Correct velocity and transfrom from server
			Velocity = ServerState.Velocity;
			VehicleMesh->SetWorldLocationAndRotation(ServerState.Transform.GetLocation(), ServerState.Transform.GetRotation());

			// 2. Simulate the unrecognized moves
			for (const FVehicleMove& Move : ClientMoves)
			{
				if (Move.Time > ServerState.LastMove.Time)
				{
					SimulateMove(Move);
				}
			}

			// 3. Empty the moves
			ClientMoves.Empty();

			// 4. Partially correct the location.
			FVector NewLocation = FMath::VInterpTo(ClientLocation, VehicleMesh->GetComponentLocation(), ClientMove.DeltaTime, NumMoves / 10.f);
			VehicleMesh->SetWorldLocation(NewLocation);
		}
	}
	else if (GetOwner()->GetLocalRole() == ROLE_SimulatedProxy)
	{
		// I don't want it to update when the time is too small
		if (ClientTimeSinceUpdate < KINDA_SMALL_NUMBER)
		{
			return;
		}

		ClientTimeBetweenLastUpdates = ClientTimeSinceUpdate;
		ClientTimeSinceUpdate = 0;

		if (VehicleMesh != nullptr)
		{
			ClientStartTransform.SetLocation(VehicleMesh->GetComponentLocation());
			ClientStartTransform.SetRotation(VehicleMesh->GetComponentQuat());
			ClientStartVelocity = Velocity;
		}
	}
}

void UVehicleMovement::ClientTick(const float& DeltaTime)
{
	ClientTimeSinceUpdate += DeltaTime;

	if (ClientTimeBetweenLastUpdates < KINDA_SMALL_NUMBER)
	{
		return;
	}

	// Derivative = DeltaLocation / DeltaAlpha
		// DeltaLocation = Velocity * dt
		// DeltaAlpha = dt / TimeBetweenLastUpdates
	// Derivative = Velocity * TimeBetweenLastUpdates;

	float Alpha = ClientTimeSinceUpdate / ClientTimeBetweenLastUpdates;
	Alpha = FMath::Clamp(Alpha, 0.f, 1.f);
	FVector StartLocation = ClientStartTransform.GetLocation();
	FVector StartDerivative = ClientStartVelocity * (ClientTimeBetweenLastUpdates * 100.f); // x100 to convert meter to centimeter
	FVector TargetLocation = ServerState.Transform.GetLocation();
	FVector TargetDerivative = ServerState.Velocity * (ClientTimeBetweenLastUpdates * 100.f);

	// Rotation
	FQuat StartRotation = ClientStartTransform.GetRotation();
	FQuat TargetRotation = ServerState.Transform.GetRotation();
	FQuat NewRotation = FQuat::Slerp(StartRotation, TargetRotation, Alpha);

	// Velocity
	FVector FirstDerivative = FMath::CubicInterpDerivative(StartLocation, StartDerivative, TargetLocation, TargetDerivative, Alpha);
	FVector NewVelocity = FirstDerivative / (ClientTimeBetweenLastUpdates * 100.f);
	Velocity = NewVelocity;

	// Acceleration
	FVector SecondDerivative = FMath::CubicInterpSecondDerivative(StartLocation, StartDerivative, TargetLocation, TargetDerivative, Alpha);
	FVector NewAcceleration = SecondDerivative / (ClientTimeBetweenLastUpdates * 100.f);

	// Location
	//FVector NewLocation = FMath::CubicInterp(StartLocation, StartDerivative, TargetLocation, TargetDerivative, Alpha);
	FVector NewLocation = VehicleMesh->GetComponentLocation() + (Velocity * DeltaTime * 100.f) + (NewAcceleration * DeltaTime * DeltaTime * 0.5f);

	if (VehicleMesh != nullptr)
	{
		VehicleMesh->SetWorldLocationAndRotation(NewLocation, NewRotation);
	}
}
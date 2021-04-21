// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VehicleMovement.generated.h"

USTRUCT()
struct FVehicleMove
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	float Throttle;
	UPROPERTY()
	float SteeringThrow;
	UPROPERTY()
	bool bIsDrifting;

	UPROPERTY()
	float DeltaTime;
	UPROPERTY()
	float Time;
};

USTRUCT()
struct FVehicleState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FVehicleMove LastMove;
};

class UStaticMeshComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GWANG_FPS_API UVehicleMovement : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVehicleMovement();

	float GetMass() const { return MassInKilo; }
	float GetElast() const { return Elast; }
	FVector GetVelocity() const { return Velocity; }
	void SetThrottle(const float& Value) { Throttle = Value; }
	void SetSteeringThrow(const float& Value) { SteeringThrow = Value; }
	void SetIsDrifting(const bool& Value) { bIsDrifting = Value; }
	void SetVehicleMesh(UStaticMeshComponent* Mesh) { VehicleMesh = Mesh; }
	void SetVelocity(const FVector& InVelocity) { Velocity = InVelocity; }

protected:
	UPROPERTY(EditAnywhere, meta = (ToolTip = "Mass of this vehicle (kg)"))
	float MassInKilo = 1000;

	UPROPERTY(EditAnywhere, meta = (ToolTip = "0(Inelastic) ~ 1(Elastic)"))
	float Elast = 1;

	UPROPERTY(EditAnywhere, meta = (ToolTip = "Force applied when the throttle is fully down (N = kgm/s^2)"))
	float MaxDrivingForce = 25000;

	UPROPERTY(EditAnywhere, meta = (ToolTip = "Minimum radius of the car turning circle at full lock (m)"))
	float MinTurningRadius = 20;

	UPROPERTY(EditAnywhere, meta = (ToolTip = "Higher is more drag (kg/m)"))
	float DragCoefficient = 16;

	UPROPERTY(EditAnywhere, meta = (ToolTip = "Higher is more rolling resistance (kg/m)"))
	float RollingResistanceCoefficient = 0.015;

	UPROPERTY(EditAnywhere, meta = (ToolTip = "Gets multiplied by mass. it won't exceed SideFriction's length"))
	float MaxSideFriction = 10;

	UPROPERTY(EditAnywhere, meta = (ToolTip = "Should be 0 < x <= 1. Decides how fast the velocy should rotate on drift"))
	float SteeringnessOnDrift = .3f;

	UPROPERTY(EditAnywhere, meta = (ToolTip = "Higher is sharper turn on drift."))
	float DriftPower = 2.f;

	float Throttle;
	float SteeringThrow;
	FVector Acceleration;
	FVector Velocity;

	// Used for ROLE_AutonomousProxy
	FVehicleMove ClientMove;
	TArray<FVehicleMove> ClientMoves;

	// Used for ROLE_SimulatedProxy
	float ClientTimeSinceUpdate;
	float ClientTimeBetweenLastUpdates;
	FTransform ClientStartTransform;
	FVector ClientStartVelocity;
	void ClientTick(const float& DeltaTime);

	// Mesh to move
	UStaticMeshComponent* VehicleMesh;

	// Drifting
	bool bIsDrifting;
	float CurrentRotationAngle;

protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Resistances getters
	FVector GetAirResistance() const;
	FVector GetRollingResistance() const;
	FVector GetSideFriction() const;

	// Movement
	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	FVehicleMove CreateMove(const float& DeltaTime);
	void SimulateMove(const FVehicleMove& Move);
	void UpdateLocationFromVelocity(const float& DeltaTime);
	void ApplyRotation(const float& DeltaTime, const float& Steering, const bool& Drift);

	// Server
	UFUNCTION(Server, Reliable)
	void Server_SendMove(const FVehicleMove& Move);
	// Called after Server_SendMove
	void UpdateServerState(const FVehicleMove& Move);

	// ServerState
	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FVehicleState ServerState;
	UFUNCTION()
	void OnRep_ServerState();

	// Collision
	void HandleCollision(const FHitResult& Hit);

};

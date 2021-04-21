// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicle/VehicleBase.h"

#include "Components/VehicleMovement.h"

// Sets default values
AVehicleBase::AVehicleBase()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VehicleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VehicleMesh"));
	RootComponent = VehicleMesh;

	VehicleMovementComp = CreateDefaultSubobject<UVehicleMovement>(TEXT("VehicleMovementComp"));

	SetReplicatingMovement(false);
	bReplicates = true;

	//NetUpdateFrequency = 5.f;
}

// Called when the game starts or when spawned
void AVehicleBase::BeginPlay()
{
	Super::BeginPlay();

	VehicleMovementComp->SetVehicleMesh(VehicleMesh);
}

// Called every frame
void AVehicleBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AVehicleBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AVehicleBase::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AVehicleBase::MoveRight);

	PlayerInputComponent->BindAction("Drift", IE_Pressed, this, &AVehicleBase::DriftPressed);
	PlayerInputComponent->BindAction("Drift", IE_Released, this, &AVehicleBase::DriftReleased);
}

void AVehicleBase::MoveForward(float Value)
{
	VehicleMovementComp->SetThrottle(Value);
}

void AVehicleBase::MoveRight(float Value)
{
	VehicleMovementComp->SetSteeringThrow(Value);
}

void AVehicleBase::DriftPressed()
{
	VehicleMovementComp->SetIsDrifting(true);
}

void AVehicleBase::DriftReleased()
{
	VehicleMovementComp->SetIsDrifting(false);
}


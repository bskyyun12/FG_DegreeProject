// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "VehicleBase.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UVehicleMovement;

UCLASS()
class GWANG_FPS_API AVehicleBase : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AVehicleBase();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* VehicleMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UVehicleMovement* VehicleMovementComp;
	 

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void DriftPressed();
	void DriftReleased();
};

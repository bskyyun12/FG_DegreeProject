// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/FPSWeaponBase.h"
#include "FPSGrenadeBase.generated.h"

USTRUCT(BlueprintType)
struct FGrenadeMove
{
	GENERATED_BODY()

	UPROPERTY()
	FVector LaunchPoint;
	UPROPERTY()
	FVector LaunchForward;
	UPROPERTY()
	FVector NewPoint;
	UPROPERTY()
	FVector PrevPoint;
	UPROPERTY()
	float CurrentSpeed;	
	UPROPERTY()
	float LaunchAngleInRad;
	UPROPERTY()
	float FlightTime;
	UPROPERTY()
	float LifeTime;
	UPROPERTY()
	float DeltaSeconds;
};

UCLASS()
class GWANG_FPS_API AFPSGrenadeBase : public AFPSWeaponBase
{
	GENERATED_BODY()
	
public:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	AFPSGrenadeBase();
	
	void Server_OnWeaponEquipped_Implementation(AFPSCharacter* OwnerCharacter) override;
	void OnRep_Owner() override;

	void Server_OnBeginFireWeapon_Implementation() override;
	void Server_OnEndFireWeapon_Implementation() override;

	void Client_OnBeginFireWeapon_Implementation() override;
	void Client_OnEndFireWeapon_Implementation() override;
	
	void Tick(float DeltaSeconds) override;



protected:
	const float GRAVITY = 981.f;

	UPROPERTY(EditDefaultsOnly)
	float ExplodeDelay = 3.f;

	UPROPERTY(EditDefaultsOnly)
	float LaunchSpeed = 3000.f;

	UPROPERTY(EditDefaultsOnly)
	float TrajectoryDrawingDeltaSeconds = 0.05f;

	UPROPERTY(EditDefaultsOnly)
	float ExplodeRadius = 550.f;

	UPROPERTY(Replicated)
	bool bSimulateGrenadeMove;

	UPROPERTY(ReplicatedUsing=OnRep_ServerMove)
	FGrenadeMove ServerMove;
	UFUNCTION()
	void OnRep_ServerMove();

	FGrenadeMove ClientMove;

	UWorld* World;
	bool bDrawtrajectory;
	bool bIsLocallyControlled;
	USkeletalMeshComponent* ArmMesh;
	USkeletalMeshComponent* CharacterMesh;





protected:
	void BeginPlay() override;

	FGrenadeMove InitializeTrajectory(float DeltaSeconds);

	UFUNCTION(Server, Unreliable)
	void Server_UpdateMoveState(FGrenadeMove Move, bool bShouldServerSimulateMove);

	void DrawGrenadePath();

	void CalcTrajectory(FGrenadeMove& Move);

	void MoveGrenade(FGrenadeMove& Move);
};

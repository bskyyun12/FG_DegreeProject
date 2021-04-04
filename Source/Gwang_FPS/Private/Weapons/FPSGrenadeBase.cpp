// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/FPSGrenadeBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

#include "FPSCharacterInterface.h"

void AFPSGrenadeBase::BeginPlay()
{
	Super::BeginPlay();
	WeaponInfo.WeaponType = EWeaponType::Grenade;
}

void AFPSGrenadeBase::Server_OnBeginFireWeapon_Implementation()
{
	//Super::Server_OnBeginFireWeapon_Implementation();
}

void AFPSGrenadeBase::Server_OnEndFireWeapon_Implementation()
{
	Super::Server_OnEndFireWeapon_Implementation();
}

void AFPSGrenadeBase::Client_OnBeginFireWeapon_Implementation()
{
	//Super::Client_OnBeginFireWeapon_Implementation();

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	if (GetOwner() != nullptr && GetOwner()->GetInstigatorController() != nullptr)
	{
		// TODO: Grenade launcher needs grenade components and pool a grenade from the components and so on
		World->GetTimerManager().SetTimer(GrenadePathTimer, this, &AFPSGrenadeBase::DrawGrenadePath, World->GetDeltaSeconds(), true);
	}
}

void AFPSGrenadeBase::Client_OnEndFireWeapon_Implementation()
{
	//Super::Client_OnEndFireWeapon_Implementation();

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	World->GetTimerManager().ClearTimer(GrenadePathTimer);
}

void AFPSGrenadeBase::DrawGrenadePath()
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	LaunchPoint = FPWeaponMesh->GetComponentLocation() + GetOwner()->GetActorForwardVector();
	PrevPoint = LaunchPoint;
	LaunchForward = GetOwner()->GetActorForwardVector();
	LaunchUp = GetOwner()->GetActorUpVector();
	float CurrentSpeed = LaunchSpeed;
	LaunchAngleInRad = GetOwner()->GetInstigatorController()->GetControlRotation().Pitch * PI / 180.f;
	FlightTime = 0.f;

	float DeltaTime = World->GetDeltaSeconds();
	for (float Time = 0.f; Time < 1.f; Time += PathDrawInSeconds)
	{
		FlightTime += PathDrawInSeconds;
		float DisplacementX = CurrentSpeed * FlightTime * FMath::Cos(LaunchAngleInRad);
		float DisplacementZ = CurrentSpeed * FlightTime * FMath::Sin(LaunchAngleInRad) - 0.5f * GRAVITY * FlightTime * FlightTime;
		FVector NewPoint = LaunchPoint + LaunchForward * DisplacementX + LaunchUp * DisplacementZ;

		DrawDebugLine(World, PrevPoint, NewPoint, FColor::Cyan, false, -1.f, 0, 7.f);

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		Params.AddIgnoredActor(this->GetOwner());
		if (World->LineTraceSingleByChannel(Hit, PrevPoint, NewPoint, ECC_Visibility, Params))
		{
			DrawDebugPoint(World, Hit.ImpactPoint, 30.f, FColor::Orange);

			FVector Reflection = FMath::GetReflectionVector((Hit.ImpactPoint - PrevPoint).GetSafeNormal(), Hit.ImpactNormal);
			FVector RightVector = FVector::CrossProduct(Reflection, LaunchUp);
			LaunchForward = FVector::CrossProduct(LaunchUp, RightVector);

			LaunchAngleInRad = FMath::Acos(FVector::DotProduct(Reflection, LaunchForward));
			LaunchAngleInRad *= FMath::Sign(Reflection.Z);

			DrawDebugLine(World, Hit.ImpactPoint, Hit.ImpactPoint + Reflection * 150.f, FColor::Purple, false, -1.f, 0, 7.f);
			DrawDebugLine(World, Hit.ImpactPoint, Hit.ImpactPoint + LaunchForward * 150.f, FColor::Red, false, -1.f, 0, 7.f);

			LaunchPoint = Hit.ImpactPoint + Hit.ImpactNormal;
			NewPoint = LaunchPoint;

			FlightTime = 0.f;

			CurrentSpeed *= 0.75f;
		}

		PrevPoint = NewPoint;
	}
}

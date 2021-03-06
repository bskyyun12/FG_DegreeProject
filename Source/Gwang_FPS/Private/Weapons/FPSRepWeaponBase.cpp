// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/FPSRepWeaponBase.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h" // DoesImplementInterface

#include "./FPSCharacterInterface.h"

// Sets default values
AFPSRepWeaponBase::AFPSRepWeaponBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SphereCollider = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SphereCollider->SetupAttachment(WeaponMesh);
	SphereCollider->OnComponentBeginOverlap.AddDynamic(this, &AFPSRepWeaponBase::OnBeginOverlap);

	SetReplicates(true);
}

void AFPSRepWeaponBase::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
	{
		if (UKismetSystemLibrary::DoesImplementInterface(OtherActor, UFPSCharacterInterface::StaticClass()))
		{
			if (IFPSCharacterInterface::Execute_HasWeapon(OtherActor, WeaponType))
			{
				UE_LOG(LogTemp, Warning, TEXT("Player: %s already has a rifle"), *OtherActor->GetName());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Player: %s pick up %s"), *OtherActor->GetName(), *this->GetName());
				IFPSCharacterInterface::Execute_EquipWeapon(OtherActor, WeaponType);
			}
		}
	}
}
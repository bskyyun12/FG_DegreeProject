// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/FPSWeaponBase.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
AFPSWeaponBase::AFPSWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	this->SetRootComponent(WeaponMesh);
}

// Called when the game starts or when spawned
void AFPSWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
}
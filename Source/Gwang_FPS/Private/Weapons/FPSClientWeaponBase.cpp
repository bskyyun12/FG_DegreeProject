// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/FPSClientWeaponBase.h"

// Sets default values
AFPSClientWeaponBase::AFPSClientWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void AFPSClientWeaponBase::OnWeaponEquipped()
{
	Super::OnWeaponEquipped();
	UE_LOG(LogTemp, Warning, TEXT("AFPSClientWeaponBase::OnWeaponEquipped()"));
}

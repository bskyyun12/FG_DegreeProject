// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/FPSKinfeBase.h"

void AFPSKinfeBase::BeginPlay()
{
	Super::BeginPlay();

	WeaponInfo.WeaponType = EWeaponType::Melee;
}

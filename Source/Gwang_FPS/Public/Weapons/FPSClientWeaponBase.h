// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/FPSWeaponBase.h"
#include "FPSClientWeaponBase.generated.h"

UCLASS()
class GWANG_FPS_API AFPSClientWeaponBase : public AFPSWeaponBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSClientWeaponBase();

public:
	void OnWeaponEquipped() override;

};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/FPSWeaponBase.h"
#include "FPSKinfeBase.generated.h"

/**
 * 
 */
UCLASS()
class GWANG_FPS_API AFPSKinfeBase : public AFPSWeaponBase
{
	GENERATED_BODY()
	
protected:
	void BeginPlay() override;

};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/FPSWeaponBase.h"
#include "FPSRepWeaponBase.generated.h"

class USphereComponent;

UCLASS()
class GWANG_FPS_API AFPSRepWeaponBase : public AFPSWeaponBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSRepWeaponBase();

protected:
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY(VisibleAnywhere)
	USphereComponent* SphereCollider;
};

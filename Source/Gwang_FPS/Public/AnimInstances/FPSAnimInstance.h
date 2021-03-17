// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "FPSAnimInterface.h"
#include "FPSAnimInstance.generated.h"


UCLASS()
class GWANG_FPS_API UFPSAnimInstance : public UAnimInstance, public IFPSAnimInterface
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CameraPitch;

public:
	void UpdateSpineAngle_Implementation(float NewPitch) override;

};

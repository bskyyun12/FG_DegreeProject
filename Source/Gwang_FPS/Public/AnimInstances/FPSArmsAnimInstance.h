// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "FPSAnimInterface.h"
#include "FPSArmsAnimInstance.generated.h"


UCLASS()
class GWANG_FPS_API UFPSArmsAnimInstance : public UAnimInstance, public IFPSAnimInterface
{
	GENERATED_BODY()
	
public:
	void UpdateBlendPose_Implementation(int NewIndex) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int BlendIndex;
};

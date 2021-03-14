// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimInstances/FPSAnimInstance.h"

void UFPSAnimInstance::UpdateBlendPose_Implementation(int NewIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("UFPSAnimInstance::UpdateBlendPose_Implementation"));
	UE_LOG(LogTemp, Warning, TEXT("New Index: %i"), NewIndex);
	BlendIndex = NewIndex;
}

void UFPSAnimInstance::UpdateSpineAngle_Implementation(float NewPitch)
{
	CameraPitch = NewPitch;
}

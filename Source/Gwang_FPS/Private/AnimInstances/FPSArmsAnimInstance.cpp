// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimInstances/FPSArmsAnimInstance.h"

void UFPSArmsAnimInstance::UpdateBlendPose_Implementation(int NewIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("UFPSArmsAnimInstance::UpdateBlendPose_Implementation"));
	UE_LOG(LogTemp, Warning, TEXT("New Index: %i"), NewIndex);
	BlendIndex = NewIndex;
}

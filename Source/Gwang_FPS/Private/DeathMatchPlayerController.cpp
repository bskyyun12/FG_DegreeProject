// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchPlayerController.h"

void ADeathMatchPlayerController::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchPlayerController::BeginPlay"));
}

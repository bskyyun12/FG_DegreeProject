// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DeathMatchPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class GWANG_FPS_API ADeathMatchPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	void BeginPlay() override;

};

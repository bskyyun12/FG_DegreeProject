// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PlayerStateInterface.h"
#include "FPSPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class GWANG_FPS_API AFPSPlayerState : public APlayerState, public IPlayerStateInterface
{
	GENERATED_BODY()

};

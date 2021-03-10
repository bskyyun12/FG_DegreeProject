// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "FPSPlayerStart.generated.h"

UENUM(BlueprintType)
enum class ETeam : uint8 {
	Dark	UMETA(DisplayName = "Team Dark"),
	Silver	UMETA(DisplayName = "Team Silver"),
};

UCLASS()
class GWANG_FPS_API AFPSPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	ETeam Team;
};

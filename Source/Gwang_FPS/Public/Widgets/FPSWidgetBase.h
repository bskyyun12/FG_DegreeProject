// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPSWidgetBase.generated.h"

class APlayerController;
class AFPSGameMode;

UCLASS()
class GWANG_FPS_API UFPSWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

	virtual void Setup();

	virtual void Teardown();

protected:
	APlayerController* OwningPlayer;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VignetteWidget.generated.h"

class UBorder;

UCLASS()
class GWANG_FPS_API UVignetteWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	bool Initialize() override;

	void OnTakeDamage();

	void SimulateVignetteEffect();

private:
	UPROPERTY(meta = (BindWidget))
	UBorder* Border_Vignette;

	UPROPERTY(EditDefaultsOnly)
	float StartAlpha = .2f;

	FTimerHandle VignetteTimer;

};

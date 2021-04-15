// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/VignetteWidget.h"
#include "Components/Border.h"

bool UVignetteWidget::Initialize()
{
	Super::Initialize();

	Border_Vignette->BrushColor.A = 0.f;

	return true;
}

void UVignetteWidget::OnTakeDamage()
{
	Border_Vignette->BrushColor.A = StartAlpha;
	Border_Vignette->SetBrushColor(Border_Vignette->BrushColor);

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	World->GetTimerManager().SetTimer(VignetteTimer, this, &UVignetteWidget::SimulateVignetteEffect, World->GetDeltaSeconds(), true);
}

void UVignetteWidget::SimulateVignetteEffect()
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	Border_Vignette->BrushColor.A = FMath::LerpStable(Border_Vignette->BrushColor.A, 0.f, World->GetDeltaSeconds());
	Border_Vignette->SetBrushColor(Border_Vignette->BrushColor);

	if (FMath::IsNearlyEqual(Border_Vignette->BrushColor.A, 0.f, .01f))
	{
		Border_Vignette->BrushColor.A = 0.f;
		Border_Vignette->SetBrushColor(Border_Vignette->BrushColor);
		World->GetTimerManager().ClearTimer(VignetteTimer);
	}
}

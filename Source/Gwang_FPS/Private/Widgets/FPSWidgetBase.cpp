// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/FPSWidgetBase.h"

bool UFPSWidgetBase::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success)
	{
		return false;
	}

	OwningPlayer = GetOwningPlayer();
	if (!ensure(OwningPlayer != nullptr))
	{
		return false;
	}

	return true;
}

void UFPSWidgetBase::Setup()
{
	this->AddToViewport();
	this->bIsFocusable = true;

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!ensure(PlayerController != nullptr))
	{
		return;
	}
	FInputModeUIOnly InputModeData;
	InputModeData.SetWidgetToFocus(this->TakeWidget());
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputModeData);

	PlayerController->bShowMouseCursor = true;
}

void UFPSWidgetBase::Teardown()
{
	this->RemoveFromViewport();
	this->bIsFocusable = false;

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!ensure(PlayerController != nullptr))
	{
		return;
	}
	FInputModeGameOnly InputModeData;
	PlayerController->SetInputMode(InputModeData);

	PlayerController->bShowMouseCursor = false;
}
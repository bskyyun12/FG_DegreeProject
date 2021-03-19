// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/FPSWidgetBase.h"

bool UFPSWidgetBase::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success)
	{
		return false;
	}

	return true;
}

void UFPSWidgetBase::Setup(EInputMode InputMode/* = EInputMode::UIOnly*/, bool bShowCursor/* = true*/)
{
	if (OwningPlayer == nullptr)
	{
		OwningPlayer = GetOwningPlayer();
		if (!ensure(OwningPlayer != nullptr))
		{
			return;
		}
	}

	this->AddToViewport();
	this->bIsFocusable = true;

	if (InputMode == EInputMode::UIOnly)
	{
		FInputModeUIOnly InputModeData;
		InputModeData.SetWidgetToFocus(this->TakeWidget());
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		OwningPlayer->SetInputMode(InputModeData);
	}
	else if (InputMode == EInputMode::GameOnly)
	{
		FInputModeGameOnly InputModeData;
		OwningPlayer->SetInputMode(InputModeData);
	}
	else if (InputMode == EInputMode::GameAndUI)
	{
		FInputModeGameAndUI InputModeData;
		InputModeData.SetWidgetToFocus(this->TakeWidget());
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
		OwningPlayer->SetInputMode(InputModeData);
	}

	OwningPlayer->bShowMouseCursor = bShowCursor;
}

void UFPSWidgetBase::Teardown()
{
	if (OwningPlayer == nullptr)
	{
		OwningPlayer = GetOwningPlayer();
		if (!ensure(OwningPlayer != nullptr))
		{
			return;
		}
	}

	this->RemoveFromViewport();
	this->bIsFocusable = false;

	FInputModeGameOnly InputModeData;
	OwningPlayer->SetInputMode(InputModeData);

	OwningPlayer->bShowMouseCursor = false;
}
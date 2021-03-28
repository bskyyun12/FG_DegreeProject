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
	this->AddToViewport();
	this->bIsFocusable = true;

	if (InputMode == EInputMode::UIOnly)
	{
		FInputModeUIOnly InputModeData;
		InputModeData.SetWidgetToFocus(this->TakeWidget());
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		GetOwningPlayer()->SetInputMode(InputModeData);
	}
	else if (InputMode == EInputMode::GameOnly)
	{
		FInputModeGameOnly InputModeData;
		GetOwningPlayer()->SetInputMode(InputModeData);
	}
	else if (InputMode == EInputMode::GameAndUI)
	{
		FInputModeGameAndUI InputModeData;
		InputModeData.SetWidgetToFocus(this->TakeWidget());
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
		GetOwningPlayer()->SetInputMode(InputModeData);
	}

	GetOwningPlayer()->bShowMouseCursor = bShowCursor;
}

void UFPSWidgetBase::Teardown()
{
	this->RemoveFromViewport();
	this->bIsFocusable = false;

	FInputModeGameOnly InputModeData;
	GetOwningPlayer()->SetInputMode(InputModeData);

	GetOwningPlayer()->bShowMouseCursor = false;
}
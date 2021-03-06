// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/TeamSelectionWidget.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h" // DoesImplementInterface

#include "FPSPlayerControllerInterface.h"

bool UTeamSelectionWidget::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success)
	{
		return false;
	}

	if (!ensure(Button_DarkTeam != nullptr))
	{
		return false;
	}
	Button_DarkTeam->OnClicked.AddDynamic(this, &UTeamSelectionWidget::OnClick_Button_DarkTeam);

	if (!ensure(Button_SilverTeam != nullptr))
	{
		return false;
	}
	Button_SilverTeam->OnClicked.AddDynamic(this, &UTeamSelectionWidget::OnClick_Button_SilverTeam);

	return true;
}

void UTeamSelectionWidget::OnClick_Button_DarkTeam()
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!ensure(PlayerController != nullptr))
	{
		return;
	}

	if (UKismetSystemLibrary::DoesImplementInterface(PlayerController, UFPSPlayerControllerInterface::StaticClass()))
	{
		IFPSPlayerControllerInterface::Execute_OnDarkTeamSelected(PlayerController);
	}
}

void UTeamSelectionWidget::OnClick_Button_SilverTeam()
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!ensure(PlayerController != nullptr))
	{
		return;
	}

	if (UKismetSystemLibrary::DoesImplementInterface(PlayerController, UFPSPlayerControllerInterface::StaticClass()))
	{
		IFPSPlayerControllerInterface::Execute_OnSilverTeamSelected(PlayerController);
	}
}

void UTeamSelectionWidget::Setup()
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

void UTeamSelectionWidget::Teardown()
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

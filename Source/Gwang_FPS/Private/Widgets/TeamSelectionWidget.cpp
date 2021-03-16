// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/TeamSelectionWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h" // DoesImplementInterface
#include "Net/UnrealNetwork.h"

#include "FPSGameMode.h"
#include "FPSPlayerControllerInterface.h"
#include "FPSPlayerStart.h"

bool UTeamSelectionWidget::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success)
	{
		return false;
	}

	if (!ensure(Button_MarvelTeam != nullptr))
	{
		return false;
	}
	Button_MarvelTeam->OnClicked.AddDynamic(this, &UTeamSelectionWidget::OnClick_Button_MarvelTeam);

	if (!ensure(Button_DCTeam != nullptr))
	{
		return false;
	}
	Button_DCTeam->OnClicked.AddDynamic(this, &UTeamSelectionWidget::OnClick_Button_DCTeam);

	return true;
}

void UTeamSelectionWidget::OnTeamFilled(ETeam Team, bool bCanJoinTeam)
{
	UE_LOG(LogTemp, Warning, TEXT("UTeamSelectionWidget::OnTeamFilled()"));
	if (Team == ETeam::Marvel)
	{
		Image_MarvelTeam->SetOpacity(bCanJoinTeam ? 1.f : 0.2f);
		Button_MarvelTeam->SetIsEnabled(bCanJoinTeam);
	}
	else if (Team == ETeam::DC)
	{
		Image_DCTeam->SetOpacity(bCanJoinTeam ? 1.f : 0.2f);
		Button_DCTeam->SetIsEnabled(bCanJoinTeam);
	}
}

void UTeamSelectionWidget::OnClick_Button_MarvelTeam()
{
	if (UKismetSystemLibrary::DoesImplementInterface(OwningPlayer, UFPSPlayerControllerInterface::StaticClass()))
	{
		IFPSPlayerControllerInterface::Execute_OnTeamSelected(OwningPlayer, ETeam::Marvel);
	}
}

void UTeamSelectionWidget::OnClick_Button_DCTeam()
{
	if (UKismetSystemLibrary::DoesImplementInterface(OwningPlayer, UFPSPlayerControllerInterface::StaticClass()))
	{
		IFPSPlayerControllerInterface::Execute_OnTeamSelected(OwningPlayer, ETeam::DC);
	}
}
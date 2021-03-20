// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/TeamSelectionWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h" // DoesImplementInterface
#include "Net/UnrealNetwork.h"

#include "FPSGameMode.h"
#include "FPSGameStateBase.h"
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

void UTeamSelectionWidget::Setup(EInputMode InputMode /*= EInputMode::UIOnly*/, bool bShowCursor /*= true*/)
{
	Super::Setup();

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	World->GetTimerManager().SetTimer(UpdateTimer, this, &UTeamSelectionWidget::RequestUIUpdate, 1.f, true);
}

void UTeamSelectionWidget::Teardown()
{
	Super::Teardown();

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	World->GetTimerManager().ClearTimer(UpdateTimer);
}


void UTeamSelectionWidget::OnClick_Button_MarvelTeam()
{
	if (OwningPlayer != nullptr && UKismetSystemLibrary::DoesImplementInterface(OwningPlayer, UFPSPlayerControllerInterface::StaticClass()))
	{
		IFPSPlayerControllerInterface::Execute_OnTeamSelected(OwningPlayer, ETeam::Marvel);
	}
}

void UTeamSelectionWidget::OnClick_Button_DCTeam()
{
	if (OwningPlayer != nullptr && UKismetSystemLibrary::DoesImplementInterface(OwningPlayer, UFPSPlayerControllerInterface::StaticClass()))
	{
		IFPSPlayerControllerInterface::Execute_OnTeamSelected(OwningPlayer, ETeam::DC);
	}
}

void UTeamSelectionWidget::RequestUIUpdate()
{
	UE_LOG(LogTemp, Warning, TEXT("UTeamSelectionWidget::RequestUIUpdate()"));
	if (OwningPlayer != nullptr && UKismetSystemLibrary::DoesImplementInterface(OwningPlayer, UFPSPlayerControllerInterface::StaticClass()))
	{
		IFPSPlayerControllerInterface::Execute_UpdateTeamSelectionUI(OwningPlayer, ETeam::DC);
	}
}

void UTeamSelectionWidget::UpdateTeamSelectionUI(ETeam Team, bool bCanJoin)
{
	UE_LOG(LogTemp, Warning, TEXT("UTeamSelectionWidget::OnUpdateTeamSelectionUI()"));
	if (Team == ETeam::Marvel)
	{
		Image_MarvelTeam->SetOpacity(bCanJoin ? 1.f : 0.2f);
		Button_MarvelTeam->SetIsEnabled(bCanJoin);
	}
	else if (Team == ETeam::DC)
	{
		Image_DCTeam->SetOpacity(bCanJoin ? 1.f : 0.2f);
		Button_DCTeam->SetIsEnabled(bCanJoin);
	}
}
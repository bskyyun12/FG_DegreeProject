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

void UTeamSelectionWidget::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UTeamSelectionWidget, bCanJoinMarvelTeam);
	DOREPLIFETIME(UTeamSelectionWidget, bCanJoinDCTeam);
}

void UTeamSelectionWidget::Setup()
{
	Super::Setup();

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	World->GetTimerManager().SetTimer(RefreshTimer, this, &UTeamSelectionWidget::Client_Refresh, 1.f, true);
}

void UTeamSelectionWidget::Teardown()
{
	Super::Teardown();
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	World->GetTimerManager().ClearTimer(RefreshTimer);
}

void UTeamSelectionWidget::Client_Refresh_Implementation()
{
	Server_Refresh();	

	Image_MarvelTeam->SetOpacity(bCanJoinMarvelTeam ? 1.f : 0.2f);
	Image_DCTeam->SetOpacity(bCanJoinDCTeam ? 1.f : 0.2f);

	Button_MarvelTeam->SetIsEnabled(bCanJoinMarvelTeam);
	Button_DCTeam->SetIsEnabled(bCanJoinDCTeam);
}

void UTeamSelectionWidget::Server_Refresh_Implementation()
{
	if (GameMode == nullptr)
	{
		UWorld* World = GetWorld();
		if (!ensure(World != nullptr))
		{
			return;
		}
		GameMode = Cast<AFPSGameMode>(UGameplayStatics::GetGameMode(World));
	}

	if (GameMode != nullptr)
	{
		bCanJoinMarvelTeam = GameMode->CanJoin(ETeam::Marvel);	// Replicated Value
		bCanJoinDCTeam = GameMode->CanJoin(ETeam::DC);	// Replicated Value
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
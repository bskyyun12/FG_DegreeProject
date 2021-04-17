// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Kismet/KismetSystemLibrary.h"

#include "MainMenu/MainMenuPlayerController.h"
#include "Lobby/UserRow.h"
#include "Lobby/LobbyPlayerController.h"
#include "Lobby/LobbyInventory.h"
#include "Lobby/LobbyPlayerControllerInterface.h"

bool ULobbyWidget::Initialize()
{
	Super::Initialize();
	UE_LOG(LogTemp, Warning, TEXT("ULobbyWidget::Initialize"));

	if (!ensure(UserRowClass != nullptr))
	{
		return false;
	}

	Button_MarvelTeam->OnClicked.AddDynamic(this, &ULobbyWidget::OnClicked_Button_MarvelTeam);
	Button_DCTeam->OnClicked.AddDynamic(this, &ULobbyWidget::OnClicked_Button_DCTeam);
	Button_BackToMainMenu->OnClicked.AddDynamic(this, &ULobbyWidget::OnClicked_Button_BackToMainMenu);
	Button_Inventory->OnClicked.AddDynamic(this, &ULobbyWidget::OnClicked_Button_Inventory);

	LobbyInventory->SetVisibility(ESlateVisibility::Hidden);

	return true;
}

// Called by ALobbyPlayerController::Client_UpdateLobbyUI_Implementation
void ULobbyWidget::UpdateUserRowData(TArray<FLobbyPlayerData> UserData)
{
	VerticalBox_TeamMarvel->ClearChildren();
	VerticalBox_TeamDC->ClearChildren();

	for (FLobbyPlayerData Data : UserData)
	{
		UWorld* World = GetWorld();
		if (!ensure(World != nullptr))
		{
			return;
		}
		UUserRow* UserRow = CreateWidget<UUserRow>(World, UserRowClass);
		if (!ensure(UserRow != nullptr))
		{
			return;
		}
		UserRow->UpdateRow(Data);

		if (Data.Team == ETeam::Marvel)
		{
			VerticalBox_TeamMarvel->AddChild(UserRow);
		}
		else if (Data.Team == ETeam::DC)
		{
			VerticalBox_TeamDC->AddChild(UserRow);
		}
	}
}

void ULobbyWidget::OnClicked_Button_MarvelTeam()
{
	UE_LOG(LogTemp, Warning, TEXT("ULobbyWidget::OnClicked_Button_MarvelTeam"));
	if (GetOwningPlayer() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetOwningPlayer(), ULobbyPlayerControllerInterface::StaticClass()))
	{
		ILobbyPlayerControllerInterface::Execute_UpdateTeamData(GetOwningPlayer(), ETeam::Marvel);
	}
}

void ULobbyWidget::OnClicked_Button_DCTeam()
{
	UE_LOG(LogTemp, Warning, TEXT("ULobbyWidget::OnClicked_Button_DCTeam"));
	if (GetOwningPlayer() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetOwningPlayer(), ULobbyPlayerControllerInterface::StaticClass()))
	{
		ILobbyPlayerControllerInterface::Execute_UpdateTeamData(GetOwningPlayer(), ETeam::DC);
	}
}

void ULobbyWidget::OnClicked_Button_BackToMainMenu()
{
	UE_LOG(LogTemp, Warning, TEXT("ULobbyWidget::OnClicked_Button_BackToMainMenu"));
	if (GetOwningPlayer() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetOwningPlayer(), ULobbyPlayerControllerInterface::StaticClass()))
	{
		ILobbyPlayerControllerInterface::Execute_LobbyToMainMenu(GetOwningPlayer());
	}
}

void ULobbyWidget::OnClicked_Button_Inventory()
{
	LobbyInventory->SetVisibility(ESlateVisibility::Visible);
}

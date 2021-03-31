// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Kismet/KismetSystemLibrary.h"

#include "MainMenu/MainMenuPlayerController.h"
#include "Lobby/UserRow.h"
#include "Lobby/LobbyPlayerController.h"
#include "Lobby/LobbyInterface.h"

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

	return true;
}

void ULobbyWidget::UpdateUserRowData(TArray<FUserData> UserData)
{
	UE_LOG(LogTemp, Warning, TEXT("ULobbyWidget::UpdateUI"));

	VerticalBox_TeamMarvel->ClearChildren();
	VerticalBox_TeamDC->ClearChildren();

	bool bIsAllReady = true;
	for (FUserData Data : UserData)
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

		if (Data.bIsReady == false)
		{
			bIsAllReady = false;
		}
	}

	if (bIsAllReady)
	{
		if (GetOwningPlayer() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetOwningPlayer(), ULobbyInterface::StaticClass()))
		{
			ILobbyInterface::Execute_StartGame(GetOwningPlayer());
		}
	}
}

void ULobbyWidget::OnClicked_Button_MarvelTeam()
{
	UE_LOG(LogTemp, Warning, TEXT("ULobbyWidget::OnClicked_Button_MarvelTeam"));
	if (GetOwningPlayer() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetOwningPlayer(), ULobbyInterface::StaticClass()))
	{
		ILobbyInterface::Execute_UpdateLobbyData(GetOwningPlayer(), ETeam::Marvel);
		ILobbyInterface::Execute_RequestLobbyUIUpdate(GetOwningPlayer());
	}
}

void ULobbyWidget::OnClicked_Button_DCTeam()
{
	UE_LOG(LogTemp, Warning, TEXT("ULobbyWidget::OnClicked_Button_DCTeam"));
	if (GetOwningPlayer() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetOwningPlayer(), ULobbyInterface::StaticClass()))
	{
		ILobbyInterface::Execute_UpdateLobbyData(GetOwningPlayer(), ETeam::DC);
		ILobbyInterface::Execute_RequestLobbyUIUpdate(GetOwningPlayer());
	}
}

void ULobbyWidget::OnClicked_Button_BackToMainMenu()
{
	UE_LOG(LogTemp, Warning, TEXT("ULobbyWidget::OnClicked_Button_BackToMainMenu"));
	if (GetOwningPlayer() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetOwningPlayer(), ULobbyInterface::StaticClass()))
	{
		ILobbyInterface::Execute_LobbyToMainMenu(GetOwningPlayer());
	}
}

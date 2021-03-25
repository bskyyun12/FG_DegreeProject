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

	Button_ReadyORStart->OnClicked.AddDynamic(this, &ULobbyWidget::OnClick_Button_ReadyORStart);

	if (!ensure(UserRowClass != nullptr))
	{
		return false;
	}

	return true;
}

void ULobbyWidget::UpdateUserRowData(TArray<FUserRowData> UserRowData)
{
	UE_LOG(LogTemp, Warning, TEXT("ULobbyWidget::UpdateUI"));

	VerticalBox_TeamMarvel->ClearChildren();
	for (FUserRowData Data : UserRowData)
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
		UserRow->SetUserName(Data.UserName.ToString());

		VerticalBox_TeamMarvel->AddChild(UserRow);
	}
}

void ULobbyWidget::OnClick_Button_ReadyORStart()
{
	UE_LOG(LogTemp, Warning, TEXT("ULobbyWidget::OnClick_Button_ReadyORStart"));
	if (GetOwningPlayer() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetOwningPlayer(), ULobbyInterface::StaticClass()))
	{
		ILobbyInterface::Execute_StartGame(GetOwningPlayer());
	}
}

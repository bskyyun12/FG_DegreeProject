// Fill out your copyright notice in the Description page of Project Settings.

#include "Lobby/UserRow.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Lobby/LobbyPlayerController.h"
#include "Lobby/LobbyPlayerControllerInterface.h"
#include "Lobby/LobbyGameMode.h"

bool UUserRow::Initialize()
{
	Super::Initialize();
	UE_LOG(LogTemp, Warning, TEXT("UUserRow::Initialize"));

	Button_Ready->OnClicked.AddDynamic(this, &UUserRow::OnClicked_Button_Ready);
	Button_Ready->SetVisibility(ESlateVisibility::Hidden);

	return true;
}

// Called by ULobbyWidget::UpdateUserRowData
void UUserRow::UpdateRow(const FLobbyPlayerData& Data)
{
	Text_UserName->SetText(FText::FromName(Data.PlayerName));

	if (Data.PlayerController == GetOwningPlayer())
	{
		Button_Ready->SetVisibility(ESlateVisibility::Visible);
	}

	bIsReady = Data.bIsReady;

	if (bIsReady)
	{
		Text_Ready->SetColorAndOpacity(TextColor_Ready);
		Text_UserName->SetColorAndOpacity(TextColor_Ready);
	}
	else
	{
		Text_Ready->SetColorAndOpacity(TextColor_Default);
		Text_UserName->SetColorAndOpacity(TextColor_Default);
	}
}

void UUserRow::OnClicked_Button_Ready()
{
	UE_LOG(LogTemp, Warning, TEXT("UUserRow::OnClicked_Button_Ready"));

	if (GetOwningPlayer() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetOwningPlayer(), ULobbyPlayerControllerInterface::StaticClass()))
	{
		bIsReady = !bIsReady;
		ILobbyPlayerControllerInterface::Execute_UpdateReadyStatus(GetOwningPlayer(), bIsReady);
	}
}

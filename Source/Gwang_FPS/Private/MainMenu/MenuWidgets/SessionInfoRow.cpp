// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu/MenuWidgets/SessionInfoRow.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

#include "MainMenu/MenuWidgets/MainMenuWidget.h"

void USessionInfoRow::InitializeRow(UMainMenuWidget* MainMenuWidget, int Index)
{
	MainMenu = MainMenuWidget;
	MainMenu->OnUpdateUI.AddDynamic(this, &USessionInfoRow::OnUpdateUI);
	RowIndex = Index;

	Button_Row->OnClicked.AddDynamic(this, &USessionInfoRow::OnClicked_Button_Row);
}

void USessionInfoRow::OnClicked_Button_Row()
{
	UE_LOG(LogTemp, Warning, TEXT("USessionInfoRow::OnClicked_Button_Row"));
	MainMenu->SetSelectIndex(RowIndex);
}

void USessionInfoRow::OnUpdateUI(int SelectedIndex)
{
	if (RowIndex == SelectedIndex)
	{
		Button_Row->SetColorAndOpacity(TextColor_Selected);
	}
	else
	{
		Button_Row->SetColorAndOpacity(TextColor_Default);
	}
}

void USessionInfoRow::SetSessionInfo(const FSessionInfoData& Data)
{
	Text_SessionName->SetText(FText::FromName(Data.ServerName));
	Text_HostName->SetText(FText::FromName(Data.HostUsername));
	Text_CurrentPlayers->SetText(FText::FromString(FString::FromInt(Data.CurrentPlayers)));
	Text_MaxPlayers->SetText(FText::FromString(FString::FromInt(Data.MaxPlayers)));
	Text_Ping->SetText(FText::FromString(FString::FromInt(Data.Ping)));

}

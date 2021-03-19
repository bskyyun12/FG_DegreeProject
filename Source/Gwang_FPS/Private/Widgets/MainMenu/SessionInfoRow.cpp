// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/MainMenu/SessionInfoRow.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

#include "Widgets/MainMenu/MainMenuWidget.h"

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

void USessionInfoRow::SetSessionName(const FString& SessionName)
{
	Text_SessionName->SetText(FText::FromString(SessionName));
}
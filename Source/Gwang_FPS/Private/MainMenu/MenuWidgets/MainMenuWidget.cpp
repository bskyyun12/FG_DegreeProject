// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu/MenuWidgets/MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/WidgetSwitcher.h"
#include "Components/EditableTextBox.h"
#include "Kismet/GameplayStatics.h"

#include "FPSGameInstance.h"
#include "MainMenu/MenuWidgets/SessionInfoRow.h"
#include "MainMenu/MenuWidgets/MainMenuInterface.h"
#include "MainMenu/MenuWidgets/MenuError.h"

void UMainMenuWidget::Setup(EInputMode InputMode /*= EInputMode::UIOnly*/, bool bShowCursor /*= true*/)
{
	Super::Setup();
	Button_Create->OnClicked.AddDynamic(this, &UMainMenuWidget::OnClicked_Button_Create);
	Button_Host->OnClicked.AddDynamic(this, &UMainMenuWidget::OnClicked_Button_Host);
	Button_Find->OnClicked.AddDynamic(this, &UMainMenuWidget::OnClicked_Button_Find);
	Button_Join->OnClicked.AddDynamic(this, &UMainMenuWidget::OnClicked_Button_Join);
	Button_BackToMenu->OnClicked.AddDynamic(this, &UMainMenuWidget::OnClicked_Button_BackToMenu);

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	FPSGameInstance = Cast<UFPSGameInstance>(UGameplayStatics::GetGameInstance(World));
	if (!ensure(FPSGameInstance != nullptr))
	{
		return;
	}

	if (!ensure(SessionInfoRowClass != nullptr))
	{
		return;
	}
}

void UMainMenuWidget::UpdateSessionList(TArray<FSessionInfoData> ServerData)
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	ScrollBox_SessionList->ClearChildren();
	for (auto& Data : ServerData)
	{
		USessionInfoRow* Row = CreateWidget<USessionInfoRow>(World, SessionInfoRowClass);
		if (!ensure(Row != nullptr))
		{
			return;
		}
		Row->InitializeRow(this, Data.Index);
		Row->SetSessionInfo(Data);
		ScrollBox_SessionList->AddChild(Row);
	}
}

void UMainMenuWidget::SetSelectIndex(int Index)
{
	SelectedIndex = Index;
	OnUpdateUI.Broadcast(SelectedIndex);
}

void UMainMenuWidget::ShowErrorWidget(const FString& Title, const FString& ErrorMsg, bool bEnableCloseButton, const float& Duration)
{
	ErrorWidget->UpdateUI(Title, ErrorMsg, bEnableCloseButton, Duration);
}

void UMainMenuWidget::OnClicked_Button_Create()
{
	UE_LOG(LogTemp, Warning, TEXT("UMainMenuWidget::OnClicked_Button_Create()"));
	WidgetSwitcher_MainMenu->SetActiveWidgetIndex(1);
}

void UMainMenuWidget::OnClicked_Button_BackToMenu()
{
	UE_LOG(LogTemp, Warning, TEXT("UMainMenuWidget::OnClicked_Button_BackToMenu()"));
	WidgetSwitcher_MainMenu->SetActiveWidgetIndex(0);
}

void UMainMenuWidget::OnClicked_Button_Host()
{
	UE_LOG(LogTemp, Warning, TEXT("UMainMenuWidget::OnClicked_Button_Host()"));

	FString SessionName = EditableText_ServerName->GetText().ToString();

	FSessionCreationData SessionCreationData;
	SessionCreationData.SessionName = *EditableText_ServerName->GetText().ToString();

	if (FPSGameInstance != nullptr && UKismetSystemLibrary::DoesImplementInterface(FPSGameInstance, UMainMenuInterface::StaticClass()))
	{
		IMainMenuInterface::Execute_Host(FPSGameInstance, SessionName);
	}
}

void UMainMenuWidget::OnClicked_Button_Find()
{
	UE_LOG(LogTemp, Warning, TEXT("UMainMenuWidget::OnClicked_Button_Find()"));
	if (FPSGameInstance != nullptr && UKismetSystemLibrary::DoesImplementInterface(FPSGameInstance, UMainMenuInterface::StaticClass()))
	{
		IMainMenuInterface::Execute_Find(FPSGameInstance);
	}
}

void UMainMenuWidget::OnClicked_Button_Join()
{
	UE_LOG(LogTemp, Warning, TEXT("UMainMenuWidget::OnClicked_Button_Join()"));
	if (SelectedIndex != -1)
	{
		UE_LOG(LogTemp, Warning, TEXT("SelectedIndex: %d"), SelectedIndex);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SelectedIndex: %d"), SelectedIndex);
	}

	if (FPSGameInstance != nullptr && UKismetSystemLibrary::DoesImplementInterface(FPSGameInstance, UMainMenuInterface::StaticClass()))
	{
		IMainMenuInterface::Execute_Join(FPSGameInstance, SelectedIndex);
	}
}

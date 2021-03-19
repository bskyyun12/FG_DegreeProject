// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/MainMenu/MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSessionSettings.h"

#include "FPSGameInstance.h"
#include "Widgets/MainMenu/MainMenuInterface.h"
#include "Widgets/MainMenu/SessionInfoRow.h"

void UMainMenuWidget::Setup(EInputMode InputMode /*= EInputMode::UIOnly*/, bool bShowCursor /*= true*/)
{
	Super::Setup();
	Button_Host->OnClicked.AddDynamic(this, &UMainMenuWidget::OnClicked_Button_Host);
	Button_Find->OnClicked.AddDynamic(this, &UMainMenuWidget::OnClicked_Button_Find);
	Button_Join->OnClicked.AddDynamic(this, &UMainMenuWidget::OnClicked_Button_Join);

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
}

void UMainMenuWidget::SetSessionInfoRowClass(TSubclassOf<UUserWidget> InSessionInfoRowClass)
{
	SessionInfoRowClass = InSessionInfoRowClass;
}

void UMainMenuWidget::UpdateSessionList(TArray<FOnlineSessionSearchResult> SearchResults)
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	ScrollBox_SessionList->ClearChildren();
	for (int i = 0; i < SearchResults.Num(); i++)
	{
		USessionInfoRow* Row = CreateWidget<USessionInfoRow>(World, SessionInfoRowClass);
		if (!ensure(Row != nullptr))
		{
			return;
		}
		Row->InitializeRow(this, i);
		Row->SetSessionName(SearchResults[i].GetSessionIdStr());
		ScrollBox_SessionList->AddChild(Row);
	}
}

void UMainMenuWidget::SetSelectIndex(int Index)
{
	SelectedIndex = Index;
	OnUpdateUI.Broadcast(SelectedIndex);
}

void UMainMenuWidget::OnClicked_Button_Host()
{
	UE_LOG(LogTemp, Warning, TEXT("UMainMenuWidget::OnClicked_Button_Host()"));
	if (FPSGameInstance != nullptr && UKismetSystemLibrary::DoesImplementInterface(FPSGameInstance, UMainMenuInterface::StaticClass()))
	{
		IMainMenuInterface::Execute_Host(FPSGameInstance);
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

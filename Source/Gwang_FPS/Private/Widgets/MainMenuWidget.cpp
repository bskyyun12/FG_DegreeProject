// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/MainMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

#include "FPSGameInstance.h"
#include "Widgets/MainMenuInterface.h"

void UMainMenuWidget::Setup(EInputMode InputMode /*= EInputMode::UIOnly*/, bool bShowCursor /*= true*/)
{
	Super::Setup();
	Button_Host->OnClicked.AddDynamic(this, &UMainMenuWidget::OnClicked_Button_Host);

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

void UMainMenuWidget::OnClicked_Button_Host()
{
	UE_LOG(LogTemp, Warning, TEXT("UMainMenuWidget::OnClicked_Button_Host()"));
	if (FPSGameInstance != nullptr && UKismetSystemLibrary::DoesImplementInterface(FPSGameInstance, UMainMenuInterface::StaticClass()))
	{
		IMainMenuInterface::Execute_Host(FPSGameInstance);
	}
}

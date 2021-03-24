// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu/MainMenuPlayerController.h"
#include "Kismet/GameplayStatics.h"

#include "MainMenu/MenuWidgets/MainMenuWidget.h"
#include "MainMenu/FPSGameInstance.h"

void AMainMenuPlayerController::Client_LoadMainMenu_Implementation()
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	if (!ensure(MainMenuWidgetClass != nullptr))
	{
		return;
	}
	MainMenu = CreateWidget<UMainMenuWidget>(World, MainMenuWidgetClass);
	if (!ensure(MainMenu != nullptr))
	{
		return;
	}
	MainMenu->Setup();

	UFPSGameInstance* FPSGameInstance = Cast<UFPSGameInstance>(UGameplayStatics::GetGameInstance(World));
	if (FPSGameInstance != nullptr)
	{
		FPSGameInstance->SetMainMenu(MainMenu);
	}
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuGameMode.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

#include "FPSGameInstance.h"

void AMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("AMainMenuGameMode::BeginPlay()"));

	if (!ensure(MainMenuWidgetClass != nullptr))
	{
		return;
	}

	if (!ensure(SessionInfoRowClass != nullptr))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	UFPSGameInstance* FPSGameInstance = Cast<UFPSGameInstance>(UGameplayStatics::GetGameInstance(World));
	if (FPSGameInstance != nullptr)
	{
		FPSGameInstance->LoadMainMenu(MainMenuWidgetClass, SessionInfoRowClass);
	}
}

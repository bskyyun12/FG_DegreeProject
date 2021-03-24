// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu/MainMenuGameMode.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

#include "MainMenu/MainMenuPlayerController.h"


void AMainMenuGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	UE_LOG(LogTemp, Warning, TEXT("AMainMenuGameMode::PostLogin"));

	AMainMenuPlayerController* MenuPlayerController = Cast<AMainMenuPlayerController>(NewPlayer);
	if (MenuPlayerController != nullptr)
	{
		MenuPlayerController->Client_LoadMainMenu();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MenuPlayerController == nullptr"));
	}
}
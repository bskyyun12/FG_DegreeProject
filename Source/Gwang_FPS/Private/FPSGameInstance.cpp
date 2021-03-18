// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSGameInstance.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "Widgets/MainMenuWidget.h"

UFPSGameInstance::UFPSGameInstance(const FObjectInitializer& ObjectIn)
{
}

void UFPSGameInstance::Init()
{
	Super::Init();
	UE_LOG(LogTemp, Warning, TEXT(" UFPSGameInstance::Init"));

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Subsystem: %s okay!"), *Subsystem->GetSubsystemName().ToString());
		SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface != nullptr)
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UFPSGameInstance::OnCreateSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UFPSGameInstance::OnDestroySessionComplete);
		}
	}
}

void UFPSGameInstance::CreateSession()
{
	if (SessionInterface != nullptr)
	{
		FOnlineSessionSettings SessionSettings;
		SessionInterface->CreateSession(0, SessionName, SessionSettings);
	}
}

void UFPSGameInstance::OnCreateSessionComplete(FName Name, bool bSuccess)
{
	UE_LOG(LogTemp, Warning, TEXT("UFPSGameInstance::OnCreateSessionComplete"));
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not create the session!"));
		return;
	}
}

void UFPSGameInstance::OnDestroySessionComplete(FName Name, bool bSuccess)
{
	UE_LOG(LogTemp, Warning, TEXT("UFPSGameInstance::OnDestroySessionComplete"));
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not destroy the session!"));
		return;
	}
	CreateSession();
}

void UFPSGameInstance::LoadMenuwidget()
{

}

void UFPSGameInstance::JoinWithIP(const FString& IPAddress)
{
	UEngine* Engine = GetEngine();
	if (!ensure(Engine != nullptr))
	{
		return;
	}
	Engine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("Joinning: %s"), *IPAddress));

	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (!ensure(PlayerController != nullptr))
	{
		return;
	}
	PlayerController->ClientTravel(IPAddress, ETravelType::TRAVEL_Absolute);
}

void UFPSGameInstance::SetMainMenuWidget(TSubclassOf<UUserWidget> MainMenuClass)
{
	UE_LOG(LogTemp, Warning, TEXT("UFPSGameInstance::SetMainMenuWidget"));
	MainMenuWidgetClass = MainMenuClass;

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	MainMenu = CreateWidget<UMainMenuWidget>(World, MainMenuWidgetClass);
	if (!ensure(MainMenu != nullptr))
	{
		return;
	}
	MainMenu->Setup();
}

void UFPSGameInstance::Host_Implementation()
{
	if (SessionInterface != nullptr)
	{
		if (SessionInterface->GetNamedSession(SessionName) != nullptr)
		{
			SessionInterface->DestroySession(SessionName);
		}
		else
		{
			CreateSession();
		}
	}
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSGameInstance.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "Widgets/MainMenu/MainMenuWidget.h"

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
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UFPSGameInstance::OnFindSessionComplete);
		}
	}
}

void UFPSGameInstance::CreateSession()
{
	if (SessionInterface != nullptr)
	{
		FOnlineSessionSettings SessionSettings;
		SessionSettings.bIsLANMatch = true;
		SessionSettings.NumPublicConnections = 6;
		SessionSettings.bShouldAdvertise = true;
		SessionInterface->CreateSession(0, SessionName, SessionSettings);
	}
}

void UFPSGameInstance::OnCreateSessionComplete(FName Name, bool bSuccess)
{
	UE_LOG(LogTemp, Warning, TEXT("UFPSGameInstance::OnCreateSessionComplete"));
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not create session!"));
		return;
	}
}

void UFPSGameInstance::OnDestroySessionComplete(FName Name, bool bSuccess)
{
	UE_LOG(LogTemp, Warning, TEXT("UFPSGameInstance::OnDestroySessionComplete"));
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not destroy session!"));
		return;
	}
	CreateSession();
}

void UFPSGameInstance::OnFindSessionComplete(bool bSuccess)
{
	UE_LOG(LogTemp, Warning, TEXT("UFPSGameInstance::OnFindSessionComplete"));
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not find session!"));
		return;
	}	

	if (SessionSearch != nullptr)
	{
		for (const FOnlineSessionSearchResult& Result : SessionSearch->SearchResults)
		{
			UE_LOG(LogTemp, Warning, TEXT("Found: %s"), *Result.GetSessionIdStr());
		}
	}
}

void UFPSGameInstance::LoadMainMenu(TSubclassOf<UUserWidget> MainMenuWidgetClass, TSubclassOf<UUserWidget> SessionInfoRowClass)
{
	UE_LOG(LogTemp, Warning, TEXT("UFPSGameInstance::SetMainMenuWidget"));

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
	MainMenu->SetSessionInfoRowClass(SessionInfoRowClass);
}

void UFPSGameInstance::Find_Implementation()
{
	if (SessionInterface != nullptr)
	{
		SessionSearch = MakeShareable(new FOnlineSessionSearch());
		SessionSearch->bIsLanQuery = true;
		if (SessionSearch != nullptr)
		{
			SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
		}
	}
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

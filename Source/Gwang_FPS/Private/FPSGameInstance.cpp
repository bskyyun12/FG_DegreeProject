// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSGameInstance.h"
#include "OnlineSessionSettings.h"

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
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UFPSGameInstance::OnJoinSessionComplete);
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

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	World->ServerTravel("/Game/FPSGame/Maps/Gwang_FPS?listen");
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
		MainMenu->UpdateSessionList(SessionSearch->SearchResults);

		// TODO: Delete the code below. It's only for debugging.
		UE_LOG(LogTemp, Warning, TEXT("Found %i sessions."), SessionSearch->SearchResults.Num());
		for (const FOnlineSessionSearchResult& Result : SessionSearch->SearchResults)
		{
			UE_LOG(LogTemp, Warning, TEXT("    > %s"), *Result.GetSessionIdStr());
		}
	}
}

void UFPSGameInstance::OnJoinSessionComplete(FName Name, EOnJoinSessionCompleteResult::Type Type)
{
	UE_LOG(LogTemp, Warning, TEXT("UFPSGameInstance::OnJoinSessionComplete"));
	switch (Type)
	{
	case EOnJoinSessionCompleteResult::Success:
		UE_LOG(LogTemp, Warning, TEXT("Success"));

		break;
	case EOnJoinSessionCompleteResult::SessionIsFull:
		UE_LOG(LogTemp, Warning, TEXT("SessionIsFull"));

		break;
	case EOnJoinSessionCompleteResult::SessionDoesNotExist:
		UE_LOG(LogTemp, Warning, TEXT("SessionDoesNotExist"));

		break;
	case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
		UE_LOG(LogTemp, Warning, TEXT("CouldNotRetrieveAddress"));

		break;
	case EOnJoinSessionCompleteResult::AlreadyInSession:
		UE_LOG(LogTemp, Warning, TEXT("AlreadyInSession"));

		break;
	case EOnJoinSessionCompleteResult::UnknownError:
		UE_LOG(LogTemp, Warning, TEXT("UnknownError"));

		break;
	default:
		break;
	}

	if (SessionInterface != nullptr)
	{
		FString Address;
		if (SessionInterface->GetResolvedConnectString(SessionName, Address))
		{
			APlayerController* PlayerController = GetFirstLocalPlayerController();
			PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
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

void UFPSGameInstance::Join_Implementation(int SessionindexToJoin)
{
	if (SessionInterface != nullptr)
	{
		if (SessionSearch != nullptr)
		{
			SessionInterface->JoinSession(0, SessionName, SessionSearch->SearchResults[SessionindexToJoin]);
		}
	}
}

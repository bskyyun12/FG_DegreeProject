// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSGameInstance.h"
#include "OnlineSessionSettings.h"
#include "GameFramework/PlayerController.h"

#include "MainMenu/MenuWidgets/MainMenuWidget.h"

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
		UE_LOG(LogTemp, Warning, TEXT("Found Subsystem: %s"), *Subsystem->GetSubsystemName().ToString());
		SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface != nullptr)
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UFPSGameInstance::OnCreateSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UFPSGameInstance::OnDestroySessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UFPSGameInstance::OnFindSessionComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UFPSGameInstance::OnJoinSessionComplete);
		}
	}

	UEngine* Engine = GetEngine();
	if (Engine != nullptr)
	{
		Engine->NetworkFailureEvent.AddUObject(this, &UFPSGameInstance::OnNetworkFailure);
	}
}

void UFPSGameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type Type, const FString& ErrorMsg)
{
	UEngine* Engine = GetEngine();
	if (Engine != nullptr)
	{
		Engine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, ErrorMsg);
	}
	DestroySession();
}

void UFPSGameInstance::Host_Implementation(const FString& InServerName)
{
	UE_LOG(LogTemp, Warning, TEXT("UFPSGameInstance::Host_Implementation"));
	ServerName = InServerName;
	if (SessionInterface != nullptr)
	{
		if (SessionInterface->GetNamedSession(SessionName) != nullptr)
		{
			DestroySession();
			if (MainMenu != nullptr)
			{
				MainMenu->ShowErrorWidget("Host Session", "Session name is already in use. Trying to destroy the session...");
			}
			// TODO: Create Session on complete destroy session?
		}
		else
		{
			CreateSession();
		}
	}
}

void UFPSGameInstance::CreateSession()
{
	UE_LOG(LogTemp, Warning, TEXT("UFPSGameInstance::CreateSession"));
	if (SessionInterface != nullptr)
	{
		FOnlineSessionSettings SessionSettings;

		SessionSettings.bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
		SessionSettings.bShouldAdvertise = true;
		SessionSettings.NumPublicConnections = 6;
		SessionSettings.bIsDedicated = false;
		SessionSettings.bUsesPresence = true;

		// One of the two booleans below allows the third steam person to join in the lobby!
		SessionSettings.bAllowJoinInProgress = true;	// TODO: Try bAllowJoinInProgress = false; and see if the third steam user can join the session
		SessionSettings.bAllowJoinViaPresence = true;	// Probably this one is in charge of allowing the third person to join!

		SessionSettings.Set(ServerNameSettingsKey, ServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

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
	World->ServerTravel("/Game/Maps/Lobby?listen");
}

void UFPSGameInstance::OnDestroySessionComplete(FName Name, bool bSuccess)
{
	UE_LOG(LogTemp, Warning, TEXT("UFPSGameInstance::OnDestroySessionComplete"));
	if (!bSuccess)
	{
		return;
	}	
}

void UFPSGameInstance::Find_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("UFPSGameInstance::Find_Implementation"));
	if (SessionInterface != nullptr)
	{
		SessionSearch = MakeShareable(new FOnlineSessionSearch());

		SessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
		SessionSearch->MaxSearchResults = 100;
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

		if (SessionSearch != nullptr)
		{
			SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
		}
	}	
	
	if (MainMenu != nullptr)
	{
		MainMenu->ShowErrorWidget("", "Trying to find sessions...", false);
	}
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
		UE_LOG(LogTemp, Warning, TEXT("Found %i sessions."), SessionSearch->SearchResults.Num());
		TArray<FSessionInfoData> ServerData;
		for (int i = 0; i < SessionSearch->SearchResults.Num(); i++)
		{
			FSessionInfoData Data;
			Data.Index = i;
			Data.MaxPlayers = SessionSearch->SearchResults[i].Session.SessionSettings.NumPublicConnections;
			Data.CurrentPlayers = Data.MaxPlayers - SessionSearch->SearchResults[i].Session.NumOpenPublicConnections;
			Data.HostUsername = *SessionSearch->SearchResults[i].Session.OwningUserName;
			Data.Ping = SessionSearch->SearchResults[i].PingInMs;

			FString Name;
			if (SessionSearch->SearchResults[i].Session.SessionSettings.Get(ServerNameSettingsKey, Name))
			{
				Data.ServerName = *Name;
			}

			ServerData.Add(Data);

			UE_LOG(LogTemp, Warning, TEXT("    > %s"), *SessionSearch->SearchResults[i].GetSessionIdStr());
		}

		if (MainMenu != nullptr)
		{
			MainMenu->UpdateSessionList(ServerData);
			MainMenu->ShowErrorWidget("", "Finished Finding sessions");
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
	
	if (MainMenu != nullptr)
	{
		MainMenu->ShowErrorWidget("", "Joining the session...", false);
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
		return;

	case EOnJoinSessionCompleteResult::SessionDoesNotExist:
		UE_LOG(LogTemp, Warning, TEXT("SessionDoesNotExist"));
		return;

	case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
		UE_LOG(LogTemp, Warning, TEXT("CouldNotRetrieveAddress"));
		return;

	case EOnJoinSessionCompleteResult::AlreadyInSession:
		UE_LOG(LogTemp, Warning, TEXT("AlreadyInSession"));
		return;

	case EOnJoinSessionCompleteResult::UnknownError:
		UE_LOG(LogTemp, Warning, TEXT("UnknownError"));
		return;
	}

	if (SessionInterface != nullptr)
	{
		FString Address;
		if (SessionInterface->GetResolvedConnectString(SessionName, Address))
		{
			UE_LOG(LogTemp, Warning, TEXT("ClientTravel to %s"), *Address);
			APlayerController* PlayerController = GetFirstLocalPlayerController();
			PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
		}
	}
}

void UFPSGameInstance::SetMainMenu(UMainMenuWidget* InMainMenu)
{
	MainMenu = InMainMenu;
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
}

void UFPSGameInstance::StartSession()
{
	if (SessionInterface != nullptr)
	{
		SessionInterface->StartSession(SessionName);
	}

	UEngine* Engine = GetEngine();
	if (Engine != nullptr)
	{
		Engine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "Game Start!");
	}
}

void UFPSGameInstance::DestroySession()
{
	if (SessionInterface != nullptr)
	{
		SessionInterface->DestroySession(SessionName);
	}
}

FPlayerData UFPSGameInstance::GetUserData() const
{
	return UserData;
}

void UFPSGameInstance::SetUserData(const FPlayerData& Data)
{
	UserData = Data;
}

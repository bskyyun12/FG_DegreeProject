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
}

void UFPSGameInstance::Host_Implementation(const FString& InServerName)
{
	ServerName = InServerName;
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

void UFPSGameInstance::CreateSession()
{
	UE_LOG(LogTemp, Warning, TEXT("UFPSGameInstance::CreateSession"));
	if (SessionInterface != nullptr)
	{
		FOnlineSessionSettings SessionSettings;

		if (IOnlineSubsystem::Get()->GetSubsystemName() != "NULL")
		{
			SessionSettings.bIsLANMatch = false;
			UE_LOG(LogTemp, Warning, TEXT("SessionSettings.bIsLANMatch = false"));
		}
		else
		{
			SessionSettings.bIsLANMatch = true;
			UE_LOG(LogTemp, Warning, TEXT("SessionSettings.bIsLANMatch = true"));
		}
		SessionSettings.bShouldAdvertise = true;
		SessionSettings.NumPublicConnections = 6;
		SessionSettings.bIsDedicated = false;
		SessionSettings.bUsesPresence = true;

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
		UE_LOG(LogTemp, Warning, TEXT("Could not destroy session!"));
		return;
	}
	CreateSession();
}

void UFPSGameInstance::Find_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("UFPSGameInstance::Find_Implementation"));
	if (SessionInterface != nullptr)
	{
		SessionSearch = MakeShareable(new FOnlineSessionSearch());
		if (IOnlineSubsystem::Get()->GetSubsystemName() != "NULL")
		{
			SessionSearch->bIsLanQuery = false;
			UE_LOG(LogTemp, Warning, TEXT("SessionSearch->bIsLanQuery = false"));
		}
		else
		{
			SessionSearch->bIsLanQuery = true;
			UE_LOG(LogTemp, Warning, TEXT("SessionSearch->bIsLanQuery = true"));
		}
		SessionSearch->MaxSearchResults = 100;
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

		if (SessionSearch != nullptr)
		{
			SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
		}
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

		MainMenu->UpdateSessionList(ServerData);
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
	//if (SessionInterface != nullptr)
	//{
	//	SessionInterface->StartSession(SessionName);
	//}
}

void UFPSGameInstance::SetTeam(ETeam InTeam)
{
	Team = InTeam;
}

ETeam UFPSGameInstance::GetTeam()
{
	return Team;
}

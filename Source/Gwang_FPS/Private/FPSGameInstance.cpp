// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSGameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"

UFPSGameInstance::UFPSGameInstance(const FObjectInitializer& ObjectIn)
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
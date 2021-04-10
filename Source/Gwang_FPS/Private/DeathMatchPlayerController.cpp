// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchPlayerController.h"
#include "Net/UnrealNetwork.h"

#include "FPSGameInstance.h"
#include "DeathMatchGameMode.h"

void ADeathMatchPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ADeathMatchPlayerController::BeginPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("(GameFlow) PlayerController::BeginPlay (%s)"), *GetName());
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		GameMode = Cast<ADeathMatchGameMode>(World->GetAuthGameMode());
	}
}

void ADeathMatchPlayerController::OnPossess(APawn* aPawn)
{
	UE_LOG(LogTemp, Warning, TEXT("(GameFlow) PlayerController::OnPossess (%s)"), *GetName());
	Super::OnPossess(aPawn);

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	Client_OnPossess();
}

void ADeathMatchPlayerController::Client_OnPossess_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("(GameFlow) PlayerController::Client_OnPossess (%s)"), *GetName());
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSPlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

#include "FPSCharacter.h"
#include "FPSGameMode.h"
#include "FPSGameState.h"
#include "Widgets/ChatPanel.h"
#include "Widgets/DamageReceiveWidget.h"
#include "Widgets/FPSHUDWidget.h"
#include "Widgets/GameOverWidget.h"
#include "Widgets/ScoreBoardWidget.h"

void AFPSPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSPlayerController, Team);
}

void AFPSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	Client_SetupWidgets();
}

// Bound to GameMode->OnStartMatch
void AFPSPlayerController::OnStartMatch()
{
	Server_RequestPlayerSpawn();
	Client_LoadGameOver(false, false);
}

// Bound to GameMode->OnEndMatch
void AFPSPlayerController::OnEndMatch()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::OnEndMatch_Implementation"));
	Client_LoadGameOver(Team == GameMode->GetWinnerTeam(), true);
}

#pragma region Getters
ETeam AFPSPlayerController::GetTeam_Implementation()
{
	return Team;
}
#pragma endregion Getters

#pragma region GameMode & GameState setup
// Called by AFPSPlayerController::OnPostLogin_Implementation
void AFPSPlayerController::GameModeSetup(AFPSGameMode* FPSGameMode)
{
	if (FPSGameMode != nullptr)
	{
		GameMode = FPSGameMode;

		GameMode->OnStartMatch.AddDynamic(this, &AFPSPlayerController::OnStartMatch);
		GameMode->OnEndMatch.AddDynamic(this, &AFPSPlayerController::OnEndMatch);
	}
}

// Called by AFPSPlayerController::OnPostLogin_Implementation
void AFPSPlayerController::GameStateSetup()
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	GameState = World->GetGameState<AFPSGameState>();
	if (!ensure(GameState != nullptr))
	{
		return;
	}
	GameState->OnUpdateScore.AddDynamic(this, &AFPSPlayerController::OnUpdateScoreUI);
}
#pragma endregion GameMode & GameState setup

#pragma region PostLogin Setup
void AFPSPlayerController::OnPostLogin_Implementation(AFPSGameMode* FPSGameMode)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::OnPostLogin_Implementation"));
	if (!ensure(FPSGameMode != nullptr))
	{
		return;
	}
	GameModeSetup(FPSGameMode);
	GameStateSetup();
	Client_SetupTeam();
}

void AFPSPlayerController::Client_SetupTeam_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::Client_SetupTeam_Implementation"));
	UFPSGameInstance* GameInstance = Cast<UFPSGameInstance>(GetGameInstance());
	if (GameInstance != nullptr)
	{
		Server_SetupTeam(GameInstance->GetUserData().Team);
	}
}

void AFPSPlayerController::Server_SetupTeam_Implementation(ETeam TeamFromGameInstance)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::Server_SetupTeam_Implementation"));
	Team = TeamFromGameInstance;	// Team is a replicated variable	
	Server_RequestPlayerSpawn();
}
#pragma endregion PostLogin Setup

#pragma region Spawn & Death
void AFPSPlayerController::Server_RequestPlayerSpawn_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::Server_RequestPlayerSpawn_Implementation"));
	if (!ensure(GameMode != nullptr))
	{
		return;
	}
	Team = (Team == ETeam::None) ? GameMode->GetTeamWithLessPeople() : Team;
	GameMode->SpawnPlayer(this, Team);
}

// Called by AFPSGameMode::SpawnPlayer
void AFPSPlayerController::OnSpawnPlayer_Implementation(AFPSCharacter* SpawnedPlayer)
{
	if (HasAuthority())
	{
		this->Possess(SpawnedPlayer); // OnRep_Pawn()

		FTransform SpawnTransform = GameMode->GetRandomPlayerStarts(Team);
		SpawnedPlayer->SetActorLocation(SpawnTransform.GetLocation());
		this->ClientSetRotation(SpawnTransform.GetRotation().Rotator());

		if (GetPawn() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetPawn(), UFPSCharacterInterface::StaticClass()))
		{
			IFPSCharacterInterface::Execute_OnSpawnPlayer(GetPawn());
		}

		FInputModeGameOnly InputModeData;
		SetInputMode(InputModeData);
	}
}

// Called by FPSCharacter::OnDeath
void AFPSPlayerController::OnPlayerDeath_Implementation()
{
	if (HasAuthority())
	{
		UWorld* World = GetWorld();
		if (!ensure(World != nullptr))
		{
			return;
		}
		FTimerHandle DeathTimer;
		World->GetTimerManager().SetTimer(DeathTimer, [&]()
			{
				GameMode->OnPlayerDeath(this, Team);
			}, 4.f, false);
	}

	if (IsLocalPlayerController())
	{
		Client_OnUpdateHealthArmorUI(true);

		FInputModeUIOnly InputModeData;
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
		SetInputMode(InputModeData);
	}
}

void AFPSPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();
	if (GetPawn() != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::OnRep_Pawn (Spawn)"));
		Client_OnUpdateHealthArmorUI(false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AFPSPlayerController::OnRep_Pawn (Death)"));
	}
}
#pragma endregion Spawn & Death

#pragma region Widget
// Called by AFPSPlayerController::BeginPlay()
void AFPSPlayerController::Client_SetupWidgets_Implementation()
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	if (!ensure(GameOverWidgetClass != nullptr))
	{
		return;
	}
	GameOverWidget = CreateWidget<UGameOverWidget>(World, GameOverWidgetClass);
	if (!ensure(GameOverWidget != nullptr))
	{
		return;
	}
	GameOverWidget->Setup(EInputMode::GameOnly, false);
	GameOverWidget->SetVisibility(ESlateVisibility::Hidden);

	if (!ensure(ScoreBoardWidgetClass != nullptr))
	{
		return;
	}
	ScoreBoardWidget = CreateWidget<UScoreBoardWidget>(World, ScoreBoardWidgetClass);
	if (!ensure(ScoreBoardWidget != nullptr))
	{
		return;
	}
	ScoreBoardWidget->Setup(EInputMode::GameOnly, false);
	ScoreBoardWidget->SetVisibility(ESlateVisibility::Hidden);

	if (!ensure(DamageReceiveWidgetClass != nullptr))
	{
		return;
	}
	DamageReceiveWidget = CreateWidget<UDamageReceiveWidget>(World, DamageReceiveWidgetClass);
	if (!ensure(DamageReceiveWidget != nullptr))
	{
		return;
	}
	DamageReceiveWidget->Setup(EInputMode::GameOnly, false);
	DamageReceiveWidget->SetVisibility(ESlateVisibility::Hidden);

	if (!ensure(FPSHUDWidgetClass != nullptr))
	{
		return;
	}
	FPSHUDWidget = CreateWidget<UFPSHUDWidget>(World, FPSHUDWidgetClass);
	if (!ensure(FPSHUDWidget != nullptr))
	{
		return;
	}
	FPSHUDWidget->Setup(EInputMode::GameOnly, false);
}

void AFPSPlayerController::OnApplyDamage_Implementation()
{
	Client_OnApplyDamage();
}

void AFPSPlayerController::Client_OnApplyDamage_Implementation()
{
	if (FPSHUDWidget != nullptr)
	{
		FPSHUDWidget->OnApplyDamage();
	}
}

void AFPSPlayerController::OnTakeDamage_Implementation()
{
	Client_OnTakeDamage();
}

void AFPSPlayerController::Client_OnTakeDamage_Implementation()
{
	if (DamageReceiveWidget != nullptr)
	{
		UWorld* World = GetWorld();
		if (!ensure(World != nullptr))
		{
			return;
		}
		DamageReceiveWidget->SetVisibility(ESlateVisibility::Visible);
		World->GetTimerManager().ClearTimer(DamageReceiveTimer);
		World->GetTimerManager().SetTimer(DamageReceiveTimer, [&]()
			{
				DamageReceiveWidget->SetVisibility(ESlateVisibility::Hidden);
			}, 2.f, false);
	}
}

void AFPSPlayerController::OnUpdateHealthArmorUI_Implementation(bool bIsDead)
{
	Client_OnUpdateHealthArmorUI(bIsDead);
}

void AFPSPlayerController::Client_OnUpdateHealthArmorUI_Implementation(bool bIsDead)
{
	if (FPSHUDWidget != nullptr)
	{
		FPSHUDWidget->OnTakeDamage(bIsDead);
	}
}

void AFPSPlayerController::OnUpdateAmmoUI_Implementation(int CurrentAmmo, int RemainingAmmo)
{
	Client_OnUpdateAmmoUI(CurrentAmmo, RemainingAmmo);
}

void AFPSPlayerController::Client_OnUpdateAmmoUI_Implementation(int CurrentAmmo, int RemainingAmmo)
{
	if (FPSHUDWidget != nullptr)
	{
		FPSHUDWidget->UpdateAmmoUI(CurrentAmmo, RemainingAmmo);
	}
}

void AFPSPlayerController::OnUpdateScoreUI(int MarvelScore, int DCScore)
{
	Client_OnUpdateScoreUI(MarvelScore, DCScore);
}

void AFPSPlayerController::Client_OnUpdateScoreUI_Implementation(int MarvelScore, int DCScore)
{
	if (FPSHUDWidget != nullptr)
	{
		FPSHUDWidget->UpdateScoreUI(MarvelScore, DCScore);
	}
}

void AFPSPlayerController::ToggleScoreBoardWidget_Implementation(bool bVisible)
{
	if (ScoreBoardWidget != nullptr)
	{
		ScoreBoardWidget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void AFPSPlayerController::Client_LoadGameOver_Implementation(bool bIsWinner, bool bWidgetVisibility)
{
	if (GameOverWidget != nullptr)
	{
		GameOverWidget->SetVisibility(bWidgetVisibility ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
		GameOverWidget->SetResultText(bIsWinner);
	}
}

// Chat
void AFPSPlayerController::StartChat_Implementation()
{
	if (FPSHUDWidget != nullptr && FPSHUDWidget->GetChatPanel() != nullptr)
	{
		FPSHUDWidget->GetChatPanel()->OnStartChat();
	}
}

void AFPSPlayerController::SendChat_Implementation(const FName& Chat)
{
	UE_LOG(LogTemp, Warning, TEXT("Chat Sender: ( %s ) "), *this->GetName());
	Server_OnSendChat(*PlayerState->GetPlayerName(), Chat);
}

void AFPSPlayerController::Server_OnSendChat_Implementation(const FName& PlayerName, const FName& Chat)
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Iterator->Get();
		if (PC != nullptr && UKismetSystemLibrary::DoesImplementInterface(PC, UFPSPlayerControllerInterface::StaticClass()))
		{
			IFPSPlayerControllerInterface::Execute_OnUpdateChatUI(PC, PlayerName, Chat);
		}
	}
}

void AFPSPlayerController::OnUpdateChatUI_Implementation(const FName& PlayerName, const FName& Chat)
{
	Client_OnUpdateChatUI(PlayerName, Chat);
}

void AFPSPlayerController::Client_OnUpdateChatUI_Implementation(const FName& PlayerName, const FName& Chat)
{
	if (FPSHUDWidget != nullptr && FPSHUDWidget->GetChatPanel() != nullptr)
	{
		FPSHUDWidget->GetChatPanel()->AddChatRow(PlayerName, Chat);
	}
}

#pragma endregion Widget

#pragma region Recoil
void AFPSPlayerController::ShakeCamera_Implementation(TSubclassOf<UCameraShakeBase> CameraShake)
{
	if (CameraShake != nullptr)
	{
		ClientStartCameraShake(CameraShake);
	}
}

void AFPSPlayerController::AddControlRotation_Implementation(const FRotator& RotationToAdd)
{
	FRotator Rotation = GetControlRotation();
	SetControlRotation(Rotation + RotationToAdd);
}
#pragma endregion Recoil
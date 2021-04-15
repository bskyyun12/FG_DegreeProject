// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchPlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

#include "DeathMatchPlayerController.h"
#include "DeathMatchGameState.h"
#include "DeathMatchGameMode.h"
#include "FPSGameInstance.h"
#include "DeathMatchCharacter.h"

void ADeathMatchPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADeathMatchPlayerState, bIsReadyToJoin);

	DOREPLIFETIME(ADeathMatchPlayerState, PlayerName);
	DOREPLIFETIME(ADeathMatchPlayerState, Team);

	// Weapons
	DOREPLIFETIME(ADeathMatchPlayerState, StartWeapons);
	DOREPLIFETIME(ADeathMatchPlayerState, CurrentWeapons);
	DOREPLIFETIME(ADeathMatchPlayerState, CurrentlyHeldWeapon);

	// Health & Armor
	DOREPLIFETIME(ADeathMatchPlayerState, CurrentHealth);
	DOREPLIFETIME(ADeathMatchPlayerState, CurrentArmor);

	// ScoreBoard
	DOREPLIFETIME(ADeathMatchPlayerState, NumKills);
	DOREPLIFETIME(ADeathMatchPlayerState, NumDeaths);
	DOREPLIFETIME(ADeathMatchPlayerState, bIsDead);

	DOREPLIFETIME(ADeathMatchPlayerState, MatchTimeLeft);
	DOREPLIFETIME(ADeathMatchPlayerState, LastChat);
}

void ADeathMatchPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UFPSGameInstance* GI = GetGameInstance<UFPSGameInstance>();
	if (!ensure(GI != nullptr))
	{
		return;
	}
	Server_ReceiveData(GI->GetPlayerData());
}

void ADeathMatchPlayerState::Server_ReceiveData_Implementation(const FPlayerData& PlayerData)
{
	PlayerName = PlayerData.PlayerName;
	Team = PlayerData.Team;

	for (uint8 i = 0; i < (uint8)EWeaponType::EnumSize; i++)
	{
		CurrentWeapons.Add(nullptr);
		StartWeapons.Add(nullptr);
	}

	// Main weapon setup
	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		ADeathMatchGameMode* GM = Cast<ADeathMatchGameMode>(World->GetAuthGameMode());
		if (!ensure(GM != nullptr))
		{
			return;
		}
		FWeaponClass WeaponClass = GM->GetWeaponClass();
		switch (PlayerData.StartMainWeapon)
		{
		case EMainWeapon::M4A1:
			if (!ensure(WeaponClass.M4A1Class != nullptr))
			{
				return;
			}
			StartWeapons[1] = World->SpawnActor<AActor>(WeaponClass.M4A1Class);
			break;

		case EMainWeapon::AK47:
			if (!ensure(WeaponClass.AK47Class != nullptr))
			{
				return;
			}
			StartWeapons[1] = World->SpawnActor<AActor>(WeaponClass.AK47Class);
			break;
		}

		// Sub weapon setup
		switch (PlayerData.StartSubWeapon)
		{
		case ESubWeapon::Pistol:
			if (!ensure(WeaponClass.PistolClass != nullptr))
			{
				return;
			}
			StartWeapons[2] = World->SpawnActor<AActor>(WeaponClass.PistolClass);
			break;
		}
	}

	CurrentWeapons = StartWeapons;
	CurrentlyHeldWeapon = CurrentWeapons[1];

	bIsReadyToJoin = true;
}

AActor* ADeathMatchPlayerState::GetCurrentWeaponWithIndex(const uint8& Index) const
{
	if (Index >= CurrentWeapons.Num())
	{
		return nullptr;
	}

	return CurrentWeapons[Index];
}

// Called after ADeathMatchGameMode::SpawnPlayer
void ADeathMatchPlayerState::Server_OnSpawn_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchPlayerState::Server_OnStartMatch"));
	Multicast_OnSpawn();
}

void ADeathMatchPlayerState::Multicast_OnSpawn_Implementation()
{
	CurrentHealth = 100;
	CurrentArmor = 100;

	if (GetPlayerController() != nullptr && GetPlayerController()->IsLocalController())
	{
		GetPlayerController()->UpdateHealthArmorUI(CurrentHealth, CurrentArmor);
	}

	CurrentWeapons = StartWeapons;
	CurrentlyHeldWeapon = CurrentWeapons[1];
}

void ADeathMatchPlayerState::Server_UpdateHealthArmor_Implementation(const uint8& NewHealth, const uint8& NewArmor)
{
	CurrentHealth = NewHealth;
	CurrentArmor = NewArmor;

	Client_UpdateHealthUI(CurrentHealth, CurrentArmor);
}

void ADeathMatchPlayerState::Client_UpdateHealthUI_Implementation(const uint8& NewHealth, const uint8& NewArmor)
{
	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchPlayerState::Client_UpdateHealthUI => Player ( %s ), Role: %i, CurrentHealth: ( %i )."), *GetPawn()->GetName(), GetLocalRole(), CurrentHealth);

	if (GetPlayerController() != nullptr && GetPlayerController()->IsLocalController())
	{
		GetPlayerController()->UpdateHealthArmorUI(NewHealth, NewArmor);
	}
}

ADeathMatchPlayerController* ADeathMatchPlayerState::GetPlayerController()
{
	if (PC == nullptr)
	{
		UWorld* World = GetWorld();
		if (World != nullptr)
		{
			PC = Cast<ADeathMatchPlayerController>(UGameplayStatics::GetPlayerController(World, 0));
			if (!ensure(PC != nullptr))
			{
				return nullptr;
			}
		}
	}

	return PC;
}

void ADeathMatchPlayerState::Server_SetTeam_Implementation(const ETeam& NewTeam)
{
	Team = NewTeam;	// OnRep_PlayerInfo()
	// TODO: Send PlayerInfo to GameState and then update HUD??
}

void ADeathMatchPlayerState::Server_OnKillPlayer_Implementation()
{
	NumKills++;	// OnRep_NumKills()
}

void ADeathMatchPlayerState::OnRep_NumKills()
{
	if (GetPawn() == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("ADeathMatchPlayerState::OnRep_NumKills => GetPawn() == nullptr"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchPlayerState::OnRep_NumKills => Player ( %s ) NumKills: ( %i )."), *GetPawn()->GetName(), NumKills);
}

void ADeathMatchPlayerState::Server_OnDeath_Implementation()
{
	NumDeaths++;	// OnRep_NumDeaths()
	bIsDead = true;	// OnRep_bIsDead()
}

void ADeathMatchPlayerState::OnRep_NumDeaths()
{
	if (GetPawn() == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("ADeathMatchPlayerState::OnRep_NumDeaths => GetPawn() == nullptr"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchPlayerState::OnRep_NumDeaths => Player ( %s ) NumDeaths: ( %i )."), *GetPawn()->GetName(), NumDeaths);
}

void ADeathMatchPlayerState::OnRep_bIsDead()
{
	if (GetPawn() == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("ADeathMatchPlayerState::OnRep_bIsDead => GetPawn() == nullptr"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchPlayerState::OnRep_bIsDead => Player ( %s ) bIsDead: ( %i )."), *GetPawn()->GetName(), bIsDead);
}

void ADeathMatchPlayerState::Server_OnSendChat_Implementation(const FName& ChatContent)
{
	LastChat = ChatContent;	// OnRep_LastChat()

	if (GetPlayerController() != nullptr && GetPlayerController()->IsLocalController())
	{
		GetPlayerController()->UpdateChatUI(PlayerName, LastChat);
	}
}

void ADeathMatchPlayerState::OnRep_LastChat()
{
	if (GetPlayerController() != nullptr && GetPlayerController()->IsLocalController())
	{
		GetPlayerController()->UpdateChatUI(PlayerName, LastChat);
	}
}

void ADeathMatchPlayerState::Server_UpdateMatchTimeLeft_Implementation(const float& TimeLeft)
{
	MatchTimeLeft = TimeLeft;	// OnRep_MatchTimeLeft()
	if (GetPlayerController() != nullptr && GetPlayerController()->IsLocalController())
	{
		GetPlayerController()->UpdateMatchTimeUI(MatchTimeLeft);
	}
}

void ADeathMatchPlayerState::OnRep_MatchTimeLeft()
{
	UE_LOG(LogTemp, Warning, TEXT("(Client) PlayerState => MatchTimeLeft: %f"), MatchTimeLeft);

	if (GetPlayerController() != nullptr && GetPlayerController()->IsLocalController())
	{
		GetPlayerController()->UpdateMatchTimeUI(MatchTimeLeft);
	}
}

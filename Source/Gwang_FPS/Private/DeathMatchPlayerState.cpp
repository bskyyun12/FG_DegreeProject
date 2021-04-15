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
	DOREPLIFETIME(ADeathMatchPlayerState, StartWeaponClasses);
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
	Server_InitialDataSetup(GI->GetPlayerData());
}

void ADeathMatchPlayerState::Server_InitialDataSetup_Implementation(const FPlayerData& PlayerData)
{
	PlayerName = PlayerData.PlayerName;
	Team = PlayerData.Team;

	for (uint8 i = 0; i < (uint8)EWeaponType::EnumSize; i++)
	{
		CurrentWeapons.Add(nullptr);
		StartWeaponClasses.Add(nullptr);
	}

	// Set up Start weapon classes
	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		ADeathMatchGameMode* GM = Cast<ADeathMatchGameMode>(World->GetAuthGameMode());
		if (GM != nullptr)
		{
			FWeaponClass WeaponClass = GM->GetWeaponClass();
			switch (PlayerData.StartMainWeapon)
			{
			case EMainWeapon::M4A1:
				if (WeaponClass.M4A1Class != nullptr)
				{
					StartWeaponClasses[1] = WeaponClass.M4A1Class;
					CurrentWeapons[1] = World->SpawnActor<AActor>(WeaponClass.M4A1Class);
				}
				break;

			case EMainWeapon::AK47:
				if (WeaponClass.AK47Class)
				{
					StartWeaponClasses[1] = WeaponClass.AK47Class;
					CurrentWeapons[1] = World->SpawnActor<AActor>(WeaponClass.AK47Class);
				}
				break;
			}

			// Sub weapon setup
			switch (PlayerData.StartSubWeapon)
			{
			case ESubWeapon::Pistol:
				if (WeaponClass.PistolClass)
				{
					StartWeaponClasses[2] = WeaponClass.PistolClass;
					CurrentWeapons[2] = World->SpawnActor<AActor>(WeaponClass.PistolClass);
				}
				break;
			}
		}
	}

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
	Team = NewTeam;
	// TODO: Send PlayerInfo to GameState and then update HUD??
}

void ADeathMatchPlayerState::Server_OnKillPlayer_Implementation()
{
	NumKills++;
}

void ADeathMatchPlayerState::Server_OnDeath_Implementation()
{
	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		FTimerHandle WeaponResetTimer;
		World->GetTimerManager().SetTimer(WeaponResetTimer, [&]()
		{
			Server_ResetCurrentWeapons();
		}, 3.f, false);
	}

	NumDeaths++;
	bIsDead = true;
}

void ADeathMatchPlayerState::Server_ResetCurrentWeapons_Implementation()
{
	// Main weapon setup
	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		for (int i = 0; i < CurrentWeapons.Num(); i++)
		{
			if (CurrentWeapons[i] != nullptr)
			{
				if (CurrentWeapons[i]->GetClass() != StartWeaponClasses[i])
				{
					CurrentWeapons[i] = World->SpawnActor<AActor>(StartWeaponClasses[i]);
				}
			}
			else
			{
				if (StartWeaponClasses[i] != nullptr)
				{
					CurrentWeapons[i] = World->SpawnActor<AActor>(StartWeaponClasses[i]);
				}
			}

			if (CurrentWeapons[i] != nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("ResetCurrentWeapons => CurrentWeapons[%i]"), i, *CurrentWeapons[i]->GetName());
			}
		}
	}

	// set main weapon(1) to currently held weapon
	CurrentlyHeldWeapon = CurrentWeapons[1];
	UE_LOG(LogTemp, Warning, TEXT("ResetCurrentWeapons => CurrentWeapons[1]"), *CurrentWeapons[1]->GetName());
}

void ADeathMatchPlayerState::Server_OnSendChat_Implementation(const FName& ChatContent)
{
	LastChat = ChatContent;

	if (GetPlayerController() != nullptr && GetPlayerController()->IsLocalController())
	{
		GetPlayerController()->UpdateChatUI(PlayerName, LastChat);
	}
}

void ADeathMatchPlayerState::Server_UpdateMatchTimeLeft_Implementation(const float& TimeLeft)
{
	MatchTimeLeft = TimeLeft;
	if (GetPlayerController() != nullptr && GetPlayerController()->IsLocalController())
	{
		GetPlayerController()->UpdateMatchTimeUI(MatchTimeLeft);
	}
}
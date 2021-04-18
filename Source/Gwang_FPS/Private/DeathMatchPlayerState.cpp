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
}

// Called after ADeathMatchGameMode::PostLogin
void ADeathMatchPlayerState::Server_OnPostLogin_Implementation()
{
	Client_OnPostLogin();

	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchPlayerState::Server_OnPostLogin ( %i ) => ( %s )"), GetLocalRole(), *GetName());
}

void ADeathMatchPlayerState::Client_OnPostLogin_Implementation()
{
	UFPSGameInstance* GI = GetGameInstance<UFPSGameInstance>();
	if (!ensure(GI != nullptr))
	{
		return;
	}
	Server_InitialDataSetup(GI->GetPlayerData());
}

void ADeathMatchPlayerState::Server_InitialDataSetup_Implementation(const FPlayerData& PlayerData)
{
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
			// Main weapon setup
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

			// Melee weapon setup
			switch (PlayerData.StartMeleeWeapon)
			{
			case EMeleeWeapon::Knife:
				if (WeaponClass.KnifeClass)
				{
					StartWeaponClasses[3] = WeaponClass.KnifeClass;
					CurrentWeapons[3] = World->SpawnActor<AActor>(WeaponClass.KnifeClass);
				}
				break;
			}

			// Grenade weapon setup
			switch (PlayerData.StartGrenade)
			{
			case EGrenade::Grenade:
				if (WeaponClass.GrenadeClass)
				{
					StartWeaponClasses[4] = WeaponClass.GrenadeClass;
					CurrentWeapons[4] = World->SpawnActor<AActor>(WeaponClass.GrenadeClass);
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

ADeathMatchGameState* ADeathMatchPlayerState::GetGameState()
{
	if (GS == nullptr)
	{
		UWorld* World = GetWorld();
		if (World != nullptr)
		{
			GS = World->GetGameState<ADeathMatchGameState>();
		}
	}

	return GS;
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

void ADeathMatchPlayerState::Server_SetTeam_Implementation(const ETeam& NewTeam)
{
	Team = NewTeam;
	// TODO: Send PlayerInfo to GameState and then update HUD??
}

void ADeathMatchPlayerState::Server_OnKillPlayer_Implementation(ADeathMatchCharacter* DeadPlayer)
{
	ETeam DeadPlayerTeam = DeadPlayer->GetTeam();
	if (DeadPlayerTeam != ETeam::None && this->Team != DeadPlayerTeam)
	{
		NumKills++;
		if (GetGameState() != nullptr)
		{
			GetGameState()->AddScore(Team);
		}
	}
}

void ADeathMatchPlayerState::Server_OnDeath_Implementation()
{
	// TODO: Here I added delay because when player dies, player's current weapon will be dropped and need to wait until it finishes replicating. => could move to on rep?
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
	Multicast_OnSendChat(ChatContent);
}

void ADeathMatchPlayerState::Multicast_OnSendChat_Implementation(const FName& ChatContent)
{
	if (GetPlayerController() != nullptr && GetPlayerController()->IsLocalController())
	{
		GetPlayerController()->UpdateChatUI(*GetPlayerName(), ChatContent);
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
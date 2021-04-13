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

	DOREPLIFETIME(ADeathMatchPlayerState, PlayerName);
	DOREPLIFETIME(ADeathMatchPlayerState, Team);

	// Weapons
	DOREPLIFETIME(ADeathMatchPlayerState, StartWeapons);
	DOREPLIFETIME(ADeathMatchPlayerState, CurrentWeapons);

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
}

AActor* ADeathMatchPlayerState::GetCurrentWeaponWithIndex(const uint8& Index) const
{
	if (Index >= CurrentWeapons.Num())
	{
		return nullptr;
	}

	return CurrentWeapons[Index];
}

void ADeathMatchPlayerState::OnPostLogin()
{
	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchPlayerState::OnPostLogin => LocalRole: %i, RemoteRole: %i"), GetLocalRole(), GetRemoteRole());

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	GM = Cast<ADeathMatchGameMode>(World->GetAuthGameMode());
	if (!ensure(GM != nullptr))
	{
		return;
	}
	GM->OnStartMatch.AddDynamic(this, &ADeathMatchPlayerState::Server_OnStartMatch);

	Client_ReadData();
}

void ADeathMatchPlayerState::Client_ReadData_Implementation()
{
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

	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchPlayerState::Server_ReceiveData => StartWeapons.Num(): %i"), StartWeapons.Num());
}

void ADeathMatchPlayerState::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	// PlayerController Setup
	PC = Cast<ADeathMatchPlayerController>(UGameplayStatics::GetPlayerController(World, 0));
	if (!ensure(PC != nullptr))
	{
		return;
	}

	//// GameMode Setup
	//if (GetLocalRole() == ROLE_Authority)
	//{
	//	GM = Cast<ADeathMatchGameMode>(World->GetAuthGameMode());
	//	if (!ensure(GM != nullptr))
	//	{
	//		return;
	//	}
	//	GM->OnStartMatch.AddDynamic(this, &ADeathMatchPlayerState::Server_OnStartMatch);
	//}
}

// This is bound to ADeathMatchGameMode::OnStartMatch delegate call
void ADeathMatchPlayerState::Server_OnStartMatch_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchPlayerState::Server_OnStartMatch"));
	Multicast_OnStartMatch();
}

void ADeathMatchPlayerState::Multicast_OnStartMatch_Implementation()
{
	CurrentHealth = 100;	// OnRep_CurrentHealth()
	CurrentArmor = 100;	// OnRep_CurrentArmor()

	if (!ensure(PC != nullptr))
	{
		return;
	}
	if (PC->IsLocalController())
	{
		PC->UpdateHealthArmorUI(CurrentHealth, CurrentArmor);
	}

	// Current weapon setup
	CurrentWeapons = StartWeapons;
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
	PC->UpdateHealthArmorUI(NewHealth, NewArmor);
}

void ADeathMatchPlayerState::Server_UpdateCurrentWeapons_Implementation(const uint8& Index, AActor* NewWeapon)
{
	CurrentWeapons[Index] = NewWeapon;
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
		UE_LOG(LogTemp, Warning, TEXT("ADeathMatchPlayerState::OnRep_CurrentHealth => GetPawn() == nullptr"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchPlayerState::OnRep_CurrentHealth => Player ( %s ) NumKills: ( %i )."), *GetPawn()->GetName(), NumKills);
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
		UE_LOG(LogTemp, Warning, TEXT("ADeathMatchPlayerState::OnRep_CurrentHealth => GetPawn() == nullptr"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchPlayerState::OnRep_CurrentHealth => Player ( %s ) NumDeaths: ( %i )."), *GetPawn()->GetName(), NumDeaths);
}

void ADeathMatchPlayerState::OnRep_bIsDead()
{
	if (GetPawn() == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("ADeathMatchPlayerState::OnRep_CurrentHealth => GetPawn() == nullptr"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchPlayerState::OnRep_CurrentHealth => Player ( %s ) bIsDead: ( %i )."), *GetPawn()->GetName(), bIsDead);
}

void ADeathMatchPlayerState::Server_OnSendChat_Implementation(const FName& ChatContent)
{
	LastChat = ChatContent;	// OnRep_LastChat()

	if (PC->IsLocalController())
	{
		PC->UpdateChatUI(PlayerName, LastChat);
	}
}

void ADeathMatchPlayerState::OnRep_LastChat()
{
	if (PC->IsLocalController())
	{
		PC->UpdateChatUI(PlayerName, LastChat);
	}
}

void ADeathMatchPlayerState::Server_UpdateMatchTimeLeft_Implementation(const float& TimeLeft)
{
	MatchTimeLeft = TimeLeft;	// OnRep_MatchTimeLeft()
	PC->UpdateMatchTimeUI(MatchTimeLeft);
}

void ADeathMatchPlayerState::OnRep_MatchTimeLeft()
{
	UE_LOG(LogTemp, Warning, TEXT("(Client) PlayerState => MatchTimeLeft: %f"), MatchTimeLeft);
	PC->UpdateMatchTimeUI(MatchTimeLeft);
}

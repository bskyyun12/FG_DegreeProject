// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchPlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"

void ADeathMatchPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADeathMatchPlayerState, PlayerInfo);
}

void ADeathMatchPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	UE_LOG(LogTemp, Warning, TEXT("(GameFlow) PlayerState::PostInitializeComponents (%s)"), *GetName());

	UFPSGameInstance* GI = GetGameInstance<UFPSGameInstance>();
	if (!ensure(GI != nullptr))
	{
		return;
	}
	PlayerInfo.PlayerName = GI->GetUserData().UserName;
	PlayerInfo.Team = GI->GetUserData().Team;
}

void ADeathMatchPlayerState::Server_SetTeam_Implementation(const ETeam& NewTeam)
{
	PlayerInfo.Team = NewTeam;	// OnRep_PlayerInfo()
	// TODO: Send PlayerInfo to GameState and then update HUD??
}

void ADeathMatchPlayerState::Server_AddNumKill_Implementation()
{
	PlayerInfo.NumKills++;	// OnRep_PlayerInfo()
}

void ADeathMatchPlayerState::Server_AddNumDeath_Implementation()
{
	PlayerInfo.NumDeaths++;	// OnRep_PlayerInfo()
}

void ADeathMatchPlayerState::OnRep_PlayerInfo()
{
	UE_LOG(LogTemp, Warning, TEXT("(Client) PlayerState => Team: %i, Kills: %i, Deaths: %i"), PlayerInfo.Team, PlayerInfo.NumKills, PlayerInfo.NumDeaths);

}
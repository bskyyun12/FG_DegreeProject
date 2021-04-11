// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchPlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

#include "DeathMatchPlayerController.h"
#include "DeathMatchGameState.h"

void ADeathMatchPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADeathMatchPlayerState, PlayerInfo);
	DOREPLIFETIME(ADeathMatchPlayerState, MatchTimeLeft);
	DOREPLIFETIME(ADeathMatchPlayerState, LastChat);
}

void ADeathMatchPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
		
	if (GetLocalRole() == ROLE_Authority)
	{
		UE_LOG(LogTemp, Warning, TEXT("(GameFlow) (Server) PlayerState::PostInitializeComponents"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("(GameFlow) (Client) PlayerState::PostInitializeComponents"));
	}

	UFPSGameInstance* GI = GetGameInstance<UFPSGameInstance>();
	if (!ensure(GI != nullptr))
	{
		return;
	}
	PlayerInfo.PlayerName = GI->GetUserData().UserName;
	PlayerInfo.Team = GI->GetUserData().Team;
}

void ADeathMatchPlayerState::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	PC = Cast<ADeathMatchPlayerController>(UGameplayStatics::GetPlayerController(World, 0));
	if (!ensure(PC != nullptr))
	{
		return;
	}
}

void ADeathMatchPlayerState::Server_SetTeam_Implementation(const ETeam& NewTeam)
{
	PlayerInfo.Team = NewTeam;	// OnRep_PlayerInfo()
	// TODO: Send PlayerInfo to GameState and then update HUD??
}

void ADeathMatchPlayerState::Server_OnKillPlayer_Implementation()
{
	PlayerInfo.NumKills++;	// OnRep_PlayerInfo()
}

void ADeathMatchPlayerState::Server_OnDeath_Implementation()
{
	PlayerInfo.NumDeaths++;	// OnRep_PlayerInfo()
	PlayerInfo.bIsDead = true;
}

void ADeathMatchPlayerState::OnRep_PlayerInfo()
{
	UE_LOG(LogTemp, Warning, TEXT("(Client) PlayerState => Team: %i, Kills: %i, Deaths: %i"), PlayerInfo.Team, PlayerInfo.NumKills, PlayerInfo.NumDeaths);

}

void ADeathMatchPlayerState::Server_UpdateMatchTimeLeft_Implementation(const float& TimeLeft)
{
	MatchTimeLeft = TimeLeft;	// OnRep_MatchTimeLeft()
	PC->UpdateMatchTimeUI(MatchTimeLeft);
}

void ADeathMatchPlayerState::Server_OnSendChat_Implementation(const FName& PlayerName, const FName& ChatContent)
{
	FChat Chat;
	Chat.PlayerName = PlayerName;
	Chat.ChatContent = ChatContent;

	LastChat = Chat;	// OnRep_LastChat()

	PC->UpdateChatUI(PlayerName, ChatContent);
}

void ADeathMatchPlayerState::OnRep_LastChat()
{
	PC->UpdateChatUI(LastChat.PlayerName, LastChat.ChatContent);
}

void ADeathMatchPlayerState::OnRep_MatchTimeLeft()
{
	UE_LOG(LogTemp, Warning, TEXT("(Client) PlayerState => MatchTimeLeft: %f"), MatchTimeLeft);
	PC->UpdateMatchTimeUI(MatchTimeLeft);
}

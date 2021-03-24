// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyInterface.h"
#include "UserRow.h"
#include "LobbyPlayerController.generated.h"

class ULobbyWidget;

UCLASS()
class GWANG_FPS_API ALobbyPlayerController : public APlayerController, public ILobbyInterface
{
	GENERATED_BODY()

public:
	void StartGame_Implementation() override;


	UFUNCTION(Server, Reliable)
	void Server_UpdateLobbyUI(const TArray<FUserRowData>& UserRowData);

	UFUNCTION(Client, Reliable)
	void Client_UpdateLobbyUI(const TArray<FUserRowData>& UserRowData);

protected:
	void BeginPlay() override;

	UFUNCTION(Client, Reliable)
	void Client_LoadLobbyWidget();

	UFUNCTION(Server, Reliable)
	void Server_OnUserEntered();


private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> LobbyWidgetClass;

	ULobbyWidget* LobbyWidget;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UserRow.h"
#include "LobbyInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class ULobbyInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GWANG_FPS_API ILobbyInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetLobbyGameMode(ALobbyGameMode* LobbyGM);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetControllerID(int ID);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	int GetControllerID();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void StartGame();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void LoadLobbyWidget();




	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void UpdateLobbyUI(const TArray<FUserRowData>& UserRowData);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetIsReady(bool bIsReady);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetTeam(ETeam Team);
};

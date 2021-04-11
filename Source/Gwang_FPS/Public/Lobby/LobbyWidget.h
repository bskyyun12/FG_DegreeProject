// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/FPSWidgetBase.h"
#include "UserRow.h"

#include "LobbyWidget.generated.h"

class UUserWidget;
class UVerticalBox;
class UButton;
class ULobbyInventory;

UCLASS()
class GWANG_FPS_API ULobbyWidget : public UFPSWidgetBase
{
	GENERATED_BODY()
	
public:
	bool Initialize() override;

	void UpdateUserRowData(TArray<FPlayerData> UserData);

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> UserRowClass;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* VerticalBox_TeamMarvel;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* VerticalBox_TeamDC;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_MarvelTeam;
	UFUNCTION()
	void OnClicked_Button_MarvelTeam();

	UPROPERTY(meta = (BindWidget))
	UButton* Button_DCTeam;
	UFUNCTION()
	void OnClicked_Button_DCTeam();

	UPROPERTY(meta = (BindWidget))
	UButton* Button_BackToMainMenu;
	UFUNCTION()
	void OnClicked_Button_BackToMainMenu();

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Inventory;
	UFUNCTION()
	void OnClicked_Button_Inventory();

	UPROPERTY(meta = (BindWidget))
	ULobbyInventory* LobbyInventory;
};

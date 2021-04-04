// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyInventory.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Lobby/LobbyInterface.h"

bool ULobbyInventory::Initialize()
{
	Super::Initialize();

	Button_MainWeaponLeft->OnClicked.AddDynamic(this, &ULobbyInventory::OnClicked_Button_MainWeaponLeft);
	Button_MainWeaponRight->OnClicked.AddDynamic(this, &ULobbyInventory::OnClicked_Button_MainWeaponRight);
	Button_Apply->OnClicked.AddDynamic(this, &ULobbyInventory::OnClicked_Button_Apply);

	if (GetOwningPlayer() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetOwningPlayer(), ULobbyInterface::StaticClass()))
	{
		FUserData UserData = ILobbyInterface::Execute_GetUserData(GetOwningPlayer());
		CurrentStartMainWeaponIndex = (int)UserData.StartMainWeapon;
		CurrentStartSubWeaponIndex = (int)UserData.StartSubWeapon;
	}

	// NAME CHANGING needs a caution
	MainWeaponEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EMainWeapon"));
	SubWeaponEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("ESubWeapon"));

	Text_MainWeapon->SetText(MainWeaponEnum->GetDisplayNameTextByIndex(CurrentStartMainWeaponIndex));
	
	return true;
}

void ULobbyInventory::OnClicked_Button_MainWeaponLeft()
{
	CurrentStartMainWeaponIndex--;
	if (CurrentStartMainWeaponIndex < 0)
	{
		CurrentStartMainWeaponIndex = MainWeaponEnum->NumEnums() - 3;
	}
	Text_MainWeapon->SetText(MainWeaponEnum->GetDisplayNameTextByIndex(CurrentStartMainWeaponIndex));
}

void ULobbyInventory::OnClicked_Button_MainWeaponRight()
{
	CurrentStartMainWeaponIndex++;
	if (CurrentStartMainWeaponIndex > MainWeaponEnum->NumEnums() - 3)
	{
		CurrentStartMainWeaponIndex = 0;
	}
	Text_MainWeapon->SetText(MainWeaponEnum->GetDisplayNameTextByIndex(CurrentStartMainWeaponIndex));
}

void ULobbyInventory::OnClicked_Button_Apply()
{
	SetVisibility(ESlateVisibility::Hidden);

	if (GetOwningPlayer() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetOwningPlayer(), ULobbyInterface::StaticClass()))
	{
		FUserData UserData = ILobbyInterface::Execute_GetUserData(GetOwningPlayer());
		UserData.StartMainWeapon = (EMainWeapon)CurrentStartMainWeaponIndex;
		UserData.StartSubWeapon = (ESubWeapon)CurrentStartSubWeaponIndex;
		ILobbyInterface::Execute_UpdateUserData(GetOwningPlayer(), UserData);
	}
}

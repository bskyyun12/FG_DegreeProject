// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyInventory.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Lobby/LobbyInterface.h"
#include "FPSGameInstance.h"

bool ULobbyInventory::Initialize()
{
	Super::Initialize();

	Button_MainWeaponLeft->OnClicked.AddDynamic(this, &ULobbyInventory::OnClicked_Button_MainWeaponLeft);
	Button_MainWeaponRight->OnClicked.AddDynamic(this, &ULobbyInventory::OnClicked_Button_MainWeaponRight);

	Button_SubWeaponLeft->OnClicked.AddDynamic(this, &ULobbyInventory::OnClicked_Button_SubWeaponLeft);
	Button_SubWeaponRight->OnClicked.AddDynamic(this, &ULobbyInventory::OnClicked_Button_SubWeaponRight);

	Button_KnifeLeft->OnClicked.AddDynamic(this, &ULobbyInventory::OnClicked_Button_KnifeLeft);
	Button_KnifeRight->OnClicked.AddDynamic(this, &ULobbyInventory::OnClicked_Button_KnifeRight);

	Button_GrenadeLeft->OnClicked.AddDynamic(this, &ULobbyInventory::OnClicked_Button_GrenadeLeft);
	Button_GrenadeRight->OnClicked.AddDynamic(this, &ULobbyInventory::OnClicked_Button_GrenadeRight);

	Button_Apply->OnClicked.AddDynamic(this, &ULobbyInventory::OnClicked_Button_Apply);

	if (GetOwningPlayer() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetOwningPlayer(), ULobbyInterface::StaticClass()))
	{
		FPlayerData UserData = ILobbyInterface::Execute_GetUserData(GetOwningPlayer());
		MainWeaponIndex = (int)UserData.StartMainWeapon;
		SubWeaponIndex = (int)UserData.StartSubWeapon;
		KnifeIndex = (int)UserData.StartMeleeWeapon;
		GrenadeIndex = (int)UserData.StartGrenade;
	}

	// ENUM NAME CHANGING needs a caution
	MainWeaponEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EMainWeapon"));
	SubWeaponEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("ESubWeapon"));
	KnifeEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EKnife"));
	GrenadeEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EGrenade"));

	ChangeWeaponText(MainWeaponIndex, MainWeaponEnum, Text_MainWeapon);
	ChangeWeaponText(SubWeaponIndex, SubWeaponEnum, Text_SubWeapon);
	ChangeWeaponText(KnifeIndex, KnifeEnum, Text_Knife);
	ChangeWeaponText(GrenadeIndex, GrenadeEnum, Text_Grenade);
	
	return true;
}

void ULobbyInventory::OnClicked_Button_MainWeaponLeft()
{
	MainWeaponIndex--;
	ChangeWeaponText(MainWeaponIndex, MainWeaponEnum, Text_MainWeapon);
}

void ULobbyInventory::OnClicked_Button_MainWeaponRight()
{
	MainWeaponIndex--;
	ChangeWeaponText(MainWeaponIndex, MainWeaponEnum, Text_MainWeapon);
}

void ULobbyInventory::OnClicked_Button_SubWeaponLeft()
{
	SubWeaponIndex--;
	ChangeWeaponText(SubWeaponIndex, SubWeaponEnum, Text_SubWeapon);
}

void ULobbyInventory::OnClicked_Button_SubWeaponRight()
{
	SubWeaponIndex++;
	ChangeWeaponText(SubWeaponIndex, SubWeaponEnum, Text_SubWeapon);
}

void ULobbyInventory::OnClicked_Button_KnifeLeft()
{
	KnifeIndex--;
	ChangeWeaponText(KnifeIndex, KnifeEnum, Text_Knife);
}

void ULobbyInventory::OnClicked_Button_KnifeRight()
{
	KnifeIndex++;
	ChangeWeaponText(KnifeIndex, KnifeEnum, Text_Knife);
}

void ULobbyInventory::OnClicked_Button_GrenadeLeft()
{
	GrenadeIndex--;
	ChangeWeaponText(GrenadeIndex, GrenadeEnum, Text_Grenade);
}

void ULobbyInventory::OnClicked_Button_GrenadeRight()
{
	GrenadeIndex++;
	ChangeWeaponText(GrenadeIndex, GrenadeEnum, Text_Grenade);
}

void ULobbyInventory::ChangeWeaponText(int& Index, UEnum* WeaponEnum, UTextBlock* WeaponText)
{
	if (Index < 0)
	{
		Index = WeaponEnum->NumEnums() - 3;
	}
	else if (Index > WeaponEnum->NumEnums() - 3)
	{
		Index = 0;
	}
	WeaponText->SetText(WeaponEnum->GetDisplayNameTextByIndex(Index));
}

void ULobbyInventory::OnClicked_Button_Apply()
{
	SetVisibility(ESlateVisibility::Hidden);

	if (GetOwningPlayer() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetOwningPlayer(), ULobbyInterface::StaticClass()))
	{
		FPlayerData UserData = ILobbyInterface::Execute_GetUserData(GetOwningPlayer());
		UserData.StartMainWeapon = (EMainWeapon)MainWeaponIndex;
		UserData.StartSubWeapon = (ESubWeapon)SubWeaponIndex;
		UserData.StartMeleeWeapon = (EMeleeWeapon)KnifeIndex;
		UserData.StartGrenade = (EGrenade)GrenadeIndex;
		ILobbyInterface::Execute_UpdateUserData(GetOwningPlayer(), UserData);
	}
}

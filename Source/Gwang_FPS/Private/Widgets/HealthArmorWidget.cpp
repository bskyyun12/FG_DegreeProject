// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HealthArmorWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetTextLibrary.h"

#include "FPSCharacterInterface.h"

void UHealthArmorWidget::UpdateUI(bool bIsDead)
{
	int Health = 0;
	int Armor = 0;

	if (bIsDead == false)
	{
		if (GetOwningPlayerPawn() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetOwningPlayerPawn(), UFPSCharacterInterface::StaticClass()))
		{
			Health = (int)IFPSCharacterInterface::Execute_GetHealth(GetOwningPlayerPawn());
			Armor = (int)IFPSCharacterInterface::Execute_GetArmor(GetOwningPlayerPawn());
		}
	}

	Text_Health->SetText(UKismetTextLibrary::Conv_IntToText(Health));
	Text_Armor->SetText(UKismetTextLibrary::Conv_IntToText(Armor));
}

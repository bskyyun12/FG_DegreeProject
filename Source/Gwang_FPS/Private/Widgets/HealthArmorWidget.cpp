// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HealthArmorWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetTextLibrary.h"

#include "FPSCharacterInterface.h"

void UHealthArmorWidget::UpdateUI(bool bIsDead)
{
	float Health = 0.f;
	float Armor = 0.f;

	if (bIsDead == false)
	{
		if (GetOwningPlayerPawn() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetOwningPlayerPawn(), UFPSCharacterInterface::StaticClass()))
		{
			Health = IFPSCharacterInterface::Execute_GetHealth(GetOwningPlayerPawn());
			Armor = IFPSCharacterInterface::Execute_GetArmor(GetOwningPlayerPawn());
		}
	}

	Text_Health->SetText(UKismetTextLibrary::Conv_FloatToText(Health, ERoundingMode::HalfToZero));
	Text_Armor->SetText(UKismetTextLibrary::Conv_FloatToText(Armor, ERoundingMode::HalfToZero));	
}

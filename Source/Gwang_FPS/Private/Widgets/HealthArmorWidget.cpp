// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HealthArmorWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetTextLibrary.h"

void UHealthArmorWidget::UpdateHealthArmorUI(const uint8& Health, const uint8& Armor)
{
	Text_Health->SetText(UKismetTextLibrary::Conv_IntToText(Health));
	Text_Armor->SetText(UKismetTextLibrary::Conv_IntToText(Armor));
}

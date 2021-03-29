// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/AmmoWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetTextLibrary.h"

void UAmmoWidget::UpdateUI(int Ammo)
{
	Text_CurrentAmmo->SetText(UKismetTextLibrary::Conv_IntToText(Ammo));
}

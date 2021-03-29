// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/AmmoWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetTextLibrary.h"

void UAmmoWidget::UpdateUI(int CurrentAmmo, int RemainingAmmo)
{
	Text_CurrentAmmo->SetText(UKismetTextLibrary::Conv_IntToText(CurrentAmmo));
	Text_RemainingAmmo->SetText(UKismetTextLibrary::Conv_IntToText(RemainingAmmo));
}

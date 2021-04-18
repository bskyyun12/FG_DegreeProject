// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/ScoreBoardUserRow.h"
#include "Components/TextBlock.h"

void UScoreBoardUserRow::UpdateUserRowData(const FName& PlayerName, const uint8& Kills, const uint8& Deaths)
{
	Text_PlayerName->SetText(FText::FromName(PlayerName));
	Text_Kills->SetText(FText::AsNumber(Kills));
	Text_Deaths->SetText(FText::AsNumber(Deaths));
}

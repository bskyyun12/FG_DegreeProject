// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/ScoreBoardUserRow.h"
#include "Components/TextBlock.h"

void UScoreBoardUserRow::UpdateUserRowData(const FName& UserName, const uint8& Kills, const uint8& Deaths)
{
	Text_UserName->SetText(FText::FromName(UserName));
	Text_Kills->SetText(FText::AsNumber(Kills));
	Text_Kills->SetText(FText::AsNumber(Deaths));
}

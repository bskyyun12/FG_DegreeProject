// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/ScoreBoardWidget.h"
#include "Components/VerticalBox.h"

#include "Widgets/ScoreBoardUserRow.h"

void UScoreBoardWidget::UpdateScoreBoard(const TArray<FScoreboardData>& ScoreBoardData)
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	if (!ensure(ScoreBoardUserRowClass != nullptr))
	{
		return;
	}

	VerticalBox_MarvelUserRow->ClearChildren();
	VerticalBox_DCUserRow->ClearChildren();

	for (const FScoreboardData& Data : ScoreBoardData)
	{
		UScoreBoardUserRow* ScoreboardRow = CreateWidget<UScoreBoardUserRow>(World, ScoreBoardUserRowClass);
		ScoreboardRow->UpdateUserRowData(Data.PlayerName, Data.Kills, Data.Deaths);

		if (Data.Team == ETeam::Marvel)
		{
			VerticalBox_MarvelUserRow->AddChild(ScoreboardRow);
		}
		else if (Data.Team == ETeam::DC)
		{
			VerticalBox_DCUserRow->AddChild(ScoreboardRow);
		}
	}
}

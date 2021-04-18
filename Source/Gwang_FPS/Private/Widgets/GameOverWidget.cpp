// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GameOverWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Kismet/KismetSystemLibrary.h" // DoesImplementInterface

void UGameOverWidget::SetResultText(const bool& bVictory, const bool& bIsDraw)
{
	if (WidgetSwitcher_Result != nullptr)
	{
		if (bIsDraw)
		{
			WidgetSwitcher_Result->SetActiveWidgetIndex(3);
		}
		else
		{
			WidgetSwitcher_Result->SetActiveWidgetIndex(bVictory);
		}
	}
}
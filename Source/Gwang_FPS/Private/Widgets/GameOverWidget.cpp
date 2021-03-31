// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GameOverWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Kismet/KismetSystemLibrary.h" // DoesImplementInterface

#include "FPSPlayerControllerInterface.h"

bool UGameOverWidget::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success)
	{
		return false;
	}

	if (!ensure(Button_Restart != nullptr))
	{
		return false;
	}
	Button_Restart->OnClicked.AddDynamic(this, &UGameOverWidget::OnClick_Button_Restart);

	return true;
}

void UGameOverWidget::SetResultText(bool Victory)
{
	if (WidgetSwitcher_Result != nullptr)
	{
		WidgetSwitcher_Result->SetActiveWidgetIndex(Victory);
	}
}

void UGameOverWidget::OnClick_Button_Restart()
{
	UE_LOG(LogTemp, Warning, TEXT("UGameOverWidget::OnClick_Button_Restart"));
	Teardown();
}

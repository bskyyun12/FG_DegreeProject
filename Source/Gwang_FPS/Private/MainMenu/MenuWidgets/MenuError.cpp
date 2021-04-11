// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu/MenuWidgets/MenuError.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

bool UMenuError::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}	

	Button_Error->OnClicked.AddDynamic(this, &UMenuError::OnClick_Button_Error);

	SetVisibility(ESlateVisibility::Hidden);

	return true;
}

void UMenuError::UpdateUI(const FString& Title, const FString& ErrorMsg, bool bEnableCloseButton)
{
	SetVisibility(ESlateVisibility::Visible);
	Text_Title->SetText(FText::FromString(Title));
	Text_ErrorMsg->SetText(FText::FromString(ErrorMsg));
	Button_Error->SetIsEnabled(bEnableCloseButton);
}

void UMenuError::OnClick_Button_Error()
{
	SetVisibility(ESlateVisibility::Hidden);
}

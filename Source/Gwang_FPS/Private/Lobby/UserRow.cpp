// Fill out your copyright notice in the Description page of Project Settings.

#include "Lobby/UserRow.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

bool UUserRow::Initialize()
{
	Super::Initialize();

	Button_Ready->OnClicked.AddDynamic(this, &UUserRow::OnClicked_Button_Ready);


	return true;
}

void UUserRow::SetUserName(const FString& UserName)
{
	Text_UserName->SetText(FText::FromString(UserName));

	if (!ensure(GetOwningPlayer() != nullptr))
	{
		return;
	}
	if (GetOwningPlayer()->GetName() == UserName)
	{
		Button_Ready->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		Button_Ready->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UUserRow::OnClicked_Button_Ready()
{
	UE_LOG(LogTemp, Warning, TEXT("UUserRow::OnClicked_Button_Ready"));
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/ChatPanel.h"
#include "Components/EditableText.h"
#include "Components/ScrollBox.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetTextLibrary.h"

#include "Widgets/ChatRow.h"
#include "PlayerControllerInterface.h"

bool UChatPanel::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	EditableText_Chat->SetVisibility(ESlateVisibility::Hidden);

	return true;
}

void UChatPanel::OnStartChat()
{
	UE_LOG(LogTemp, Warning, TEXT("UChatPanel::OnStartChat"));
	if (GetOwningPlayer() != nullptr)
	{
		FInputModeGameAndUI InputModeData;
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputModeData.SetHideCursorDuringCapture(true);
		InputModeData.SetWidgetToFocus(EditableText_Chat->GetCachedWidget());
		GetOwningPlayer()->SetInputMode(InputModeData);

		GetOwningPlayer()->bShowMouseCursor = true;
	}
	
	EditableText_Chat->SetVisibility(ESlateVisibility::Visible);
	EditableText_Chat->SetText(FText::FromString(""));
}

void UChatPanel::SendChat()
{
	UE_LOG(LogTemp, Warning, TEXT("UChatPanel::SendChat"));
	FName ChatToSend = *(UKismetTextLibrary::Conv_TextToString(EditableText_Chat->GetText()));
	if (GetOwningPlayer() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetOwningPlayer(), UPlayerControllerInterface::StaticClass()))
	{
		IPlayerControllerInterface::Execute_SendChat(GetOwningPlayer(), ChatToSend);
	}

	EditableText_Chat->SetText(FText::FromString(""));
}

void UChatPanel::EndChat()
{
	UE_LOG(LogTemp, Warning, TEXT("UChatPanel::EndChat"));

	if (GetOwningPlayer() != nullptr)
	{
		FInputModeGameOnly InputModeData;
		GetOwningPlayer()->SetInputMode(InputModeData);

		GetOwningPlayer()->bShowMouseCursor = false;
	}

	EditableText_Chat->SetVisibility(ESlateVisibility::Hidden);
}

void UChatPanel::AddChatRow(const FName& PlayerName, const FName& Chat)
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	if (!ensure(ChatRowClass != nullptr))
	{
		return;
	}
	UChatRow* ChatRow = CreateWidget<UChatRow>(World, ChatRowClass);
	if (!ensure(ChatRow != nullptr))
	{
		return;
	}
	ChatRow->OnChatAdded(PlayerName, Chat);
	ScrollBox_Chat->AddChild(ChatRow);
	ScrollBox_Chat->ScrollToEnd();
}

FReply UChatPanel::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);

	if (InKeyEvent.GetKey() == EKeys::Enter)
	{
		if (EditableText_Chat->GetText().ToString() != "")
		{
			SendChat();
		}
		else
		{
			EndChat();
		}
	}
	else if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		EndChat();
	}

	return FReply::Handled();
}

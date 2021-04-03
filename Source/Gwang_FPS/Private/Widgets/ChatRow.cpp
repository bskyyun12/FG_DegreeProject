// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/ChatRow.h"

void UChatRow::OnChatAdded(const FName& UserName, const FName& Chat)
{
	Text_UserName->SetText(FText::FromName(UserName));
	Text_Chat->SetText(FText::FromName(Chat));
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/FPSWidgetBase.h"
#include "ChatRow.generated.h"

class UTextBlock;

UCLASS()
class GWANG_FPS_API UChatRow : public UFPSWidgetBase
{
	GENERATED_BODY()
	
public:
	void OnChatAdded(const FName& UserName, const FName& Chat);

private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_UserName;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Chat;
};

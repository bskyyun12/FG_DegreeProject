// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/FPSWidgetBase.h"
#include "UserRow.generated.h"

class UTextBlock;
class UButton;

USTRUCT()
struct FUserRowData
{
	GENERATED_BODY()

	UPROPERTY()
	FName UserName;

	FUserRowData()
	{
		UserName = "Gwang";
	}
};

UCLASS()
class GWANG_FPS_API UUserRow : public UFPSWidgetBase
{
	GENERATED_BODY()
	
public:
	bool Initialize() override;

	void SetUserName(const FString& UserName);

private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_UserName;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Ready;
	UFUNCTION()
	void OnClicked_Button_Ready();
};

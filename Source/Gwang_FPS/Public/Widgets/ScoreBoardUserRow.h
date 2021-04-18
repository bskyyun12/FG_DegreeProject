// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/FPSWidgetBase.h"
#include "ScoreBoardUserRow.generated.h"

class UTextBlock;

UCLASS()
class GWANG_FPS_API UScoreBoardUserRow : public UFPSWidgetBase
{
	GENERATED_BODY()

public:
	void UpdateUserRowData(const FName& PlayerName, const uint8& Kills, const uint8& Deaths);

private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_PlayerName;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Kills;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Deaths;
};

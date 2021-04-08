// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/FPSWidgetBase.h"
#include "ScoreBoardWidget.generated.h"

class UVerticalBox;

UCLASS()
class GWANG_FPS_API UScoreBoardWidget : public UFPSWidgetBase
{
	GENERATED_BODY()

public:
	void UpdateScoreBoard();

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> ScoreBoardUserRowClass;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* VerticalBox_MarvelUserRow;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* VerticalBox_DCUserRow;
};

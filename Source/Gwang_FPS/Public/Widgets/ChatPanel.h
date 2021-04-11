// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatPanel.generated.h"

class UScrollBox;
class UEditableText;

UCLASS()
class GWANG_FPS_API UChatPanel : public UUserWidget
{
	GENERATED_BODY()
	
public:
	bool Initialize() override;
	
	void OnStartChat();
	void SendChat();
	void EndChat();

	void AddChatRow(const FName& PlayerName, const FName& Chat);

private:
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ScrollBox_Chat;

	UPROPERTY(meta = (BindWidget))
	UEditableText* EditableText_Chat;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> ChatRowClass;

protected:
	FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

};

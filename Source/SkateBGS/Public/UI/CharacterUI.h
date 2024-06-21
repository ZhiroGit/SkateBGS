// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterUI.generated.h"

/**
 * 
 */
UCLASS()
class SKATEBGS_API UCharacterUI : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetStaminaPercent(float Percent);
	void UpdateRingCount(int32 Rings);
	void UpdateTimer(int32 Minutes, int32 Seconds);
	
private:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* StaminaBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* RingCount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Timer;
};

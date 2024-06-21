// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CharacterUI.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UCharacterUI::SetStaminaPercent(float Percent)
{
	if (StaminaBar)
	{
		StaminaBar->SetPercent(Percent);
	}
}

void UCharacterUI::UpdateRingCount(int32 Rings)
{
	if (RingCount)
	{
		RingCount->SetText(FText::FromString(FString::Printf(TEXT("%d"), Rings)));
	}
}

void UCharacterUI::UpdateTimer(int32 Minutes, int32 Seconds)
{
	if (Timer)
	{
		FString MinuteString;
		FString SecondString;

		if (Minutes < 10)
		{
			MinuteString = FString::Printf(TEXT("0%d"), Minutes);
		}
		else
		{
			MinuteString = FString::Printf(TEXT("%d"), Minutes);
		}

		if (Seconds < 10)
		{
			SecondString = FString::Printf(TEXT("0%d"), Seconds);
		}
		else
		{
			SecondString = FString::Printf(TEXT("%d"), Seconds);
		}

		FString TimerText = FString(MinuteString + ":" + SecondString);
		Timer->SetText(FText::FromString(TimerText));
	}
}

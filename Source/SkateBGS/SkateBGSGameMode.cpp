// Copyright Epic Games, Inc. All Rights Reserved.

#include "SkateBGSGameMode.h"
#include "SkateBGSCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASkateBGSGameMode::ASkateBGSGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

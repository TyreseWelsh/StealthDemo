// Copyright Epic Games, Inc. All Rights Reserved.

#include "StealthDemoGameMode.h"
#include "StealthDemoCharacter.h"
#include "UObject/ConstructorHelpers.h"

AStealthDemoGameMode::AStealthDemoGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Player/Blueprints/BP_Player"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

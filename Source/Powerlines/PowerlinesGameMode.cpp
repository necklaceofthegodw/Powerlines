// Copyright Epic Games, Inc. All Rights Reserved.

#include "PowerlinesGameMode.h"
#include "PowerlinesCharacter.h"
#include "UObject/ConstructorHelpers.h"

APowerlinesGameMode::APowerlinesGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

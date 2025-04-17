// Copyright Epic Games, Inc. All Rights Reserved.

#include "KimyuminDemoGameMode.h"
#include "KimyuminDemoCharacter.h"
#include "UObject/ConstructorHelpers.h"

AKimyuminDemoGameMode::AKimyuminDemoGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

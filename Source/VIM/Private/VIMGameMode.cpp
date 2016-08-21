// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "VIM.h"
#include "VIMGameMode.h"
#include "VIMPlayerController.h"
#include "VIMCharacter.h"

AVIMGameMode::AVIMGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AVIMPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDownCPP/Blueprints/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
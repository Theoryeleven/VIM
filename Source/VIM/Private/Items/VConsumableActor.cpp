// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "VIM.h"
#include "VConsumableActor.h"
#include "VCharacter.h"


AVConsumableActor::AVConsumableActor(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	/* A default to tweak per food variation in Blueprint */
	Nutrition = 20;

	bAllowRespawn = true;
	RespawnDelay = 60.0f;
	RespawnDelayRange = 20.0f;
}


void AVConsumableActor::OnUsed(APawn* InstigatorPawn)
{
	AVCharacter* Pawn = Cast<AVCharacter>(InstigatorPawn);
	if (Pawn)
	{
		/* Restore some hitpoints and energy (hunger) */
		Pawn->RestoreCondition(Nutrition * 0.5f, Nutrition);
	}

	Super::OnUsed(InstigatorPawn);
}

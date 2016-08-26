// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "World/VGameMode.h"
#include "VOpenWorldGameMode.generated.h"

/**
 * 
 */
UCLASS(ABSTRACT)
class VIM_API AVOpenWorldGameMode : public AVGameMode
{
	GENERATED_BODY()

	AVOpenWorldGameMode(const FObjectInitializer& ObjectInitializer);
	
	/* Don't allow spectating of other players or bots */
	virtual bool CanSpectate_Implementation(APlayerController* Viewer, APlayerState* ViewTarget) override;
};

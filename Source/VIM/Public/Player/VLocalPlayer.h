// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/LocalPlayer.h"
#include "VLocalPlayer.generated.h"

/**
 * LocalPlayer defines a specific user when dealing with multiple players on a single machine (eg. local split-screen)
 */
UCLASS()
class VIM_API UVLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()

	/* Set a player name if no online system like Steam is available */
	virtual FString GetNickname() const override;
};

// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/PlayerStart.h"
#include "VPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class VIM_API AVPlayerStart : public APlayerStart
{
	GENERATED_BODY()

	AVPlayerStart(const class FObjectInitializer& ObjectInitializer);

	/* Is only useable by players - automatically a preferred spawn for players */
	UPROPERTY(EditAnywhere, Category = "PlayerStart")
	bool bPlayerOnly;

public:

	bool GetIsPlayerOnly() { return bPlayerOnly; }

};

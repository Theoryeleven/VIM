// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "VCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class VIM_API UVCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	virtual float GetMaxSpeed() const override;

};
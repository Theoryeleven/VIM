// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "VIM.h"
#include "VCharacterMovementComponent.h"
#include "VBaseCharacter.h"



float UVCharacterMovementComponent::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	const AVBaseCharacter* CharOwner = Cast<AVBaseCharacter>(PawnOwner);
	
	return MaxSpeed;
}
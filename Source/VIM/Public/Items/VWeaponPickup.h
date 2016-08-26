// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "VPickupActor.h"
#include "VWeaponPickup.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class VIM_API AVWeaponPickup : public AVPickupActor
{
	GENERATED_BODY()

	AVWeaponPickup(const FObjectInitializer& ObjectInitializer);

public:

	/* Class to add to inventory when picked up */
	UPROPERTY(EditDefaultsOnly, Category = "WeaponClass")
	TSubclassOf<AVWeapon> WeaponClass;

	virtual void OnUsed(APawn* InstigatorPawn) override;
};

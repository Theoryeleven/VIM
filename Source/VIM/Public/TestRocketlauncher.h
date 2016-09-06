// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Items/VWeapon.h"
#include "TestRocketlauncher.generated.h"

/**
 * 
 */
UCLASS()
class VIM_API ATestRocketlauncher : public AVWeapon
{
	GENERATED_BODY()
	
		virtual void FireWeapon() override;

private:
	UPROPERTY(EditDefaultsOnly)
		float ImpulseStrength;
	
	
};

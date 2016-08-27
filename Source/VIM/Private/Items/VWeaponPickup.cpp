// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "VIM.h"
#include "VCharacter.h"
#include "VWeapon.h"
#include "VWeaponPickup.h"
#include "VPlayerController.h"


AVWeaponPickup::AVWeaponPickup(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	bAllowRespawn = false;

	/* Enabled to support simulated physics movement when weapons are dropped by a player */
	bReplicateMovement = true;
}


void AVWeaponPickup::OnUsed(APawn* InstigatorPawn)
{
	AVCharacter* MyPawn = Cast<AVCharacter>(InstigatorPawn);
	if (MyPawn)
	{
		/* Fetch the default variables of the class we are about to pick up and check if the storage slot is available on the pawn. */
		if (MyPawn->WeaponSlotAvailable(WeaponClass->GetDefaultObject<AVWeapon>()->GetStorageSlot()))
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AVWeapon* NewWeapon = GetWorld()->SpawnActor<AVWeapon>(WeaponClass, SpawnInfo);

			MyPawn->AddWeapon(NewWeapon);

			Super::OnUsed(InstigatorPawn);
		}
		else
		{
			AVPlayerController* PC = Cast<AVPlayerController>(MyPawn->GetController());
			if (PC)
			{
				PC->HUDMessage(EHUDMessage::Weapon_SlotTaken);
			}
		}
	}
}



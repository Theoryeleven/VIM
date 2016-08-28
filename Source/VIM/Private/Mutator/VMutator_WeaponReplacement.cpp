// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "VIM.h"
#include "VGameMode.h"
#include "VMutator_WeaponReplacement.h"



bool AVMutator_WeaponReplacement::CheckRelevance_Implementation(AActor* Other)
{
	AVWeaponPickup* WeaponPickup = Cast<AVWeaponPickup>(Other);
	if (WeaponPickup)
	{
		for (int32 i = 0; i < WeaponsToReplace.Num(); i++)
		{
			const FReplacementInfo& Info = WeaponsToReplace[i];

			if (Info.FromWeapon == WeaponPickup->WeaponClass)
			{
				WeaponPickup->WeaponClass = Info.ToWeapon;
			}
		}
	}

	/* Always call Super so we can run the entire chain of linked Mutators. */
	return Super::CheckRelevance_Implementation(Other);
}


void AVMutator_WeaponReplacement::InitGame_Implementation(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	/* Update default inventory weapons for current gamemode. */
	AVGameMode* GameMode = Cast<AVGameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode != nullptr)
	{
		for (int32 i = 0; i < GameMode->DefaultInventoryClasses.Num(); i++)
		{
			for (int32 j = 0; j < WeaponsToReplace.Num(); j++)
			{
				FReplacementInfo Info = WeaponsToReplace[j];
				if (GameMode->DefaultInventoryClasses[i] == Info.FromWeapon)
				{
					GameMode->DefaultInventoryClasses[i] = Info.ToWeapon;
				}
			}
		}
	}

	Super::InitGame_Implementation(MapName, Options, ErrorMessage);
}

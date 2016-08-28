// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Mutator/VMutator.h"
#include "VMutator_WeaponReplacement.generated.h"


USTRUCT(BlueprintType)
struct FReplacementInfo
{
	GENERATED_USTRUCT_BODY()
	/** class name of the weapon we want to get rid of */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AVWeapon> FromWeapon;
	/** fully qualified path of the class to replace it with */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AVWeapon> ToWeapon;

	FReplacementInfo()
		:FromWeapon(nullptr)
		, ToWeapon(nullptr)
	{
	}

	FReplacementInfo(TSubclassOf<AVWeapon> inOldClass, TSubclassOf<AVWeapon> inNewClass)
		: FromWeapon(inOldClass)
		, ToWeapon(inNewClass)
	{
	}

};

/**
 * Allows mutators to replace weapon pickups in the active level
 */
UCLASS(ABSTRACT)
class VIM_API AVMutator_WeaponReplacement : public AVMutator
{
	GENERATED_BODY()

public:

	virtual void InitGame_Implementation(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	virtual bool CheckRelevance_Implementation(AActor* Other) override;

	UPROPERTY(EditDefaultsOnly, Category = "WeaponReplacement")
	TArray<FReplacementInfo> WeaponsToReplace;
	
};

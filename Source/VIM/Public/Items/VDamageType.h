// copyright The Perfect Game Company 2016

#pragma once

#include "GameFramework/DamageType.h"
#include "VDamageType.generated.h"

/**
 * 
 */
UCLASS()
class VIM_API UVDamageType : public UDamageType
{
	GENERATED_BODY()
	
		

			UVDamageType(const FObjectInitializer& ObjectInitializer);

		/* Can player die from this damage type (players don't die from hunger) */
		UPROPERTY(EditDefaultsOnly)
			bool bCanDieFrom;

		/* Damage modifier for head shot damage */
		UPROPERTY(EditDefaultsOnly)
			float HeadDmgModifier;

		UPROPERTY(EditDefaultsOnly)
			float LimbDmgModifier;

	public:

		bool GetCanDieFrom();

		float GetHeadDamageModifier();

		float GetLimbDamageModifier();
};

	
	


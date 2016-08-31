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

		/* Damage modifier*/
		UPROPERTY(EditDefaultsOnly)
			float DefaultDmgModifier;

		UPROPERTY(EditDefaultsOnly)
			float BodyDmgModifier;

		UPROPERTY(EditDefaultsOnly)
			float ArmourDmgModifier;

		UPROPERTY(EditDefaultsOnly)
			float ShieldDmgModifier;

		UPROPERTY(EditDefaultsOnly)
			float InanimateDmgModifier;

	


	public:

		bool GetCanDieFrom();

		float GetDefaultDamageModifier();

		float GetBodyDamageModifier();

		float GetArmourDamageModifier();

		float GetShieldDamageModifier();

		float GetInanimateDamageModifier();
};

	
	


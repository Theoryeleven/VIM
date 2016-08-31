// copyright The Perfect Game Company 2016

#include "VIM.h"
#include "VDamageType.h"


UVDamageType::UVDamageType(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	/* We apply this modifier based on the physics material setup to Different material types */
	DefaultDmgModifier = 1.0f;
	BodyDmgModifier = 1.0f;
	ArmourDmgModifier = 0.5f;
	ShieldDmgModifier = 0.25f;
	InanimateDmgModifier = 0.125f;

	bCanDieFrom = true;
}


bool UVDamageType::GetCanDieFrom()
{
	return bCanDieFrom;
}


float UVDamageType::GetDefaultDamageModifier()
{
	return DefaultDmgModifier;
}

float UVDamageType::GetBodyDamageModifier()
{
	return BodyDmgModifier;
}

float UVDamageType::GetArmourDamageModifier()
{
	return ArmourDmgModifier;
}

float UVDamageType::GetShieldDamageModifier()
{
	return ShieldDmgModifier;
}

float UVDamageType::GetInanimateDamageModifier()
{
	return InanimateDmgModifier;
}
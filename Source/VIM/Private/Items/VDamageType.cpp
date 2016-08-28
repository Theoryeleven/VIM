// copyright The Perfect Game Company 2016

#include "VIM.h"
#include "VDamageType.h"


UVDamageType::UVDamageType(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	/* We apply this modifier based on the physics material setup to the head of the enemy PhysAsset */
	HeadDmgModifier = 2.0f;
	LimbDmgModifier = 0.5f;

	bCanDieFrom = true;
}


bool UVDamageType::GetCanDieFrom()
{
	return bCanDieFrom;
}


float UVDamageType::GetHeadDamageModifier()
{
	return HeadDmgModifier;
}

float UVDamageType::GetLimbDamageModifier()
{
	return LimbDmgModifier;
}


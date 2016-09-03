// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#ifndef __VIM_H__
#define __VIM_H__

#include "Engine.h"
// This is NOT included by default in an empty project! It's required for replication and setting of the GetLifetimeReplicatedProps
#include "Net/UnrealNetwork.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVIM, Log, All);

/** when you modify this, please note that this information can be saved with instances
* also DefaultEngine.ini [/Script/Engine.CollisionProfile] should match with this list **/
#define COLLISION_WEAPON			ECC_GameTraceChannel1
#define COLLISION_PROJECTILE		ECC_GameTraceChannel2
/** when you modify this, please note that this information can be saved with instances
* also DefaultEngine.ini [/Script/Engine.PhysicsSettings] should match with this list **/
#define SURFACE_DEFAULT				SurfaceType_Default
#define SURFACE_BODY				SurfaceType1
#define SURFACE_ARMOUR				SurfaceType2
#define SURFACE_SHIELDS			    SurfaceType3
#define SURFACE_INANIMATE			SurfaceType4

#endif

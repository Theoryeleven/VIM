// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "VIM.h"
#include "VPlayerCameraManager.h"
#include "VCharacter.h"


AVPlayerCameraManager::AVPlayerCameraManager(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// bind mouse wheel to camera control
	
}


void AVPlayerCameraManager::BeginPlay()
{
	Super::BeginPlay();

	AVCharacter* MyPawn = Cast<AVCharacter>(PCOwner->GetPawn());
	
	if (MyPawn)
	{
	
	/* Cache the offset Z value of the camera to maintain a correct offset while transition*/
 		DefaultCameraOffsetZ = MyPawn->GetCameraComponent()->GetRelativeTransform().GetLocation().Z;
 	}
	return;
}

void AVPlayerCameraManager::ZoomIn(float Val)
{
	CurrentZoomOffset = Val;
}


void AVPlayerCameraManager::UpdateCamera(float DeltaTime)
{
	AVCharacter* MyPawn = Cast<AVCharacter>(PCOwner->GetPawn());

	if (MyPawn)
	{
		FVector CurrentCameraOffset = MyPawn->GetCameraComponent()->GetRelativeTransform().GetLocation();
		FVector NewCameraOffset = FVector(CurrentCameraOffset.X, CurrentCameraOffset.Y, DefaultCameraOffsetZ + CurrentZoomOffset);
		MyPawn->GetCameraComponent()->SetRelativeLocation(NewCameraOffset);
	}

	Super::UpdateCamera(DeltaTime);
}
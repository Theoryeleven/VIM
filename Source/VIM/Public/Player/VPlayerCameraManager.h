// Copyright 2016 The Perfect Game Company

#pragma once
#include "Camera/PlayerCameraManager.h"
#include "VPlayerCameraManager.generated.h"

/**
 * Player Camera handling to allow change of view and zooming in for top down views
 */

UCLASS(Blueprintable)
class VIM_API AVPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

	
		AVPlayerCameraManager(const FObjectInitializer& ObjectInitializer);

		/* Update the FOV */
		virtual void UpdateCamera(float DeltaTime) override;

		virtual void BeginPlay() override;
		virtual void ZoomIn(float Val);
		float CurrentZoomOffset;
		/* Default relative Z offset of the player camera */
		float DefaultCameraOffsetZ;

	
};
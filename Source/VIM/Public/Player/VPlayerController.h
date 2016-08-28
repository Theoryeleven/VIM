// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/PlayerController.h"
#include "VHUD.h"
#include "VPlayerController.generated.h"

UENUM()
enum class EHUDMessage : uint8
{
	/* Weapons */
	Weapon_SlotTaken,

	/* Character */
	Character_EnergyRestored,

	/* Gamemode */
	Game_SurviveStart,
	Game_SurviveEnded,

	/* No category specified */
	None,
};

/**
 * 
 */
UCLASS()
class VIM_API AVPlayerController : public APlayerController
{
	GENERATED_BODY()

	AVPlayerController(const FObjectInitializer& ObjectInitializer);

	/* Flag to respawn or start spectating upon death */
	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	bool bRespawnImmediately;
	
	/* Respawn or start spectating after dying */
	virtual void UnFreeze() override;
protected:


public:

	

	UFUNCTION(Reliable, Client)
		void HUDStateChanged(EHUDState NewState);

	void HUDStateChanged_Implementation(EHUDState NewState);

	/* Enum is remapped to localized text before sending it to the HUD */
	UFUNCTION(Reliable, Client)
		void HUDMessage(EHUDMessage MessageID);

	void HUDMessage_Implementation(EHUDMessage MessageID);

	FText GetText(EHUDMessage MsgID);

	/* Kill the current pawn */
	UFUNCTION(exec)
		virtual void Suicide();

	/* Start spectators. Should be called only on server */
	void StartSpectating();
};

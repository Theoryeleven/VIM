// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "VIM.h"
#include "VPlayerController.h"
#include "VPlayerCameraManager.h"
#include "VCharacter.h"
#include "VBaseCharacter.h"
#include "VHUD.h"
#include "VGameState.h"


/* Define a log category for error messages */
DEFINE_LOG_CATEGORY_STATIC(LogGame, Log, All);


AVPlayerController::AVPlayerController(const class FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	/* Assign the class types we wish to use */
	PlayerCameraManagerClass = AVPlayerCameraManager::StaticClass();

	/* Example - Can be set to true for debugging, generally a value like this would exist in the GameMode instead */
	bRespawnImmediately = false;
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}



void AVPlayerController::UnFreeze()
{
	Super::UnFreeze();

	// Check if match is ending or has ended.
	AVGameState* MyGameState = Cast<AVGameState>(GetWorld()->GameState);
	if (MyGameState && MyGameState->HasMatchEnded())
	{
		/* Don't allow spectating or respawns */
		return;
	}

	/* Respawn or spectate */
	if (bRespawnImmediately)
	{
		ServerRestartPlayer();
	}
	else
	{
		StartSpectating();
	}
}


void AVPlayerController::StartSpectating()
{
	/* Update the state on server */
	PlayerState->bIsSpectator = true;
	/* Waiting to respawn */
	bPlayerIsWaiting = true;
	ChangeState(NAME_Spectating);
	/* Push the state update to the client */
	ClientGotoState(NAME_Spectating);

	/* Focus on the remaining alive player */
	ViewAPlayer(1);

	/* Update the HUD to show the spectator screen */
	HUDStateChanged(EHUDState::Spectating);
}


void AVPlayerController::Suicide()
{
	AVCharacter* MyPawn = Cast<AVCharacter>(GetPawn());
	if (!ensure(MyPawn)) { return; }
	if (MyPawn && ((GetWorld()->TimeSeconds - MyPawn->CreationTime > 1)))
	{
		MyPawn->Suicide();
	}
}

void AVPlayerController::HUDStateChanged_Implementation(EHUDState NewState)
{
	AVHUD* MyHUD = Cast<AVHUD>(GetHUD());
	if (MyHUD)
	{
		MyHUD->OnStateChanged(NewState);
	}
}


void AVPlayerController::HUDMessage_Implementation(EHUDMessage MessageID)
{
	/* Turn the ID into a message for the HUD to display */
	FText TextMessage = GetText(MessageID);

	AVHUD* MyHUD = Cast<AVHUD>(GetHUD());
	if (MyHUD)
	{
		/* Implemented in SurvivalHUD Blueprint */
		MyHUD->MessageReceived(TextMessage);
	}
}

/* Temporarily set the namespace. If this was omitted, we should call NSLOCTEXT(Namespace, x, y) instead */
#define LOCTEXT_NAMESPACE "HUDMESSAGES"

FText AVPlayerController::GetText(EHUDMessage MsgID)
{
	switch (MsgID)
	{
	case EHUDMessage::Weapon_SlotTaken:
		return LOCTEXT("WeaponSlotTaken", "Weapon slot already taken.");
	case EHUDMessage::Character_EnergyRestored:
		return LOCTEXT("CharacterEnergyRestored", "Energy Restored");
	case EHUDMessage::Game_SurviveStart:
		return LOCTEXT("GameNightStart", "SURVIVE THE NIGHT");
	case EHUDMessage::Game_SurviveEnded:
		return LOCTEXT("GameNightEnd", "Night survived! Prepare for the coming night.");
	default:
		UE_LOG(LogGame, Warning, TEXT("No Message set for enum value in SPlayerContoller::GetText(). "))
			return FText::FromString("No Message Set");
	}
}

/* Remove the namespace definition so it doesn't exist in other files compiled after this one. */
#undef LOCTEXT_NAMESPACE
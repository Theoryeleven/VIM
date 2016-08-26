// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "VIM.h"
#include "VCoopGameMode.h"



/* Define a log category for error messages */
DEFINE_LOG_CATEGORY_STATIC(LogGameMode, Log, All);



AVCoopGameMode::AVCoopGameMode(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	/* Disable damage to coop buddies  */
	bAllowFriendlyFireDamage = false;
	bSpawnAtTeamPlayer = true;
	
	ScoreNightSurvived = 1000;
}


/*
	RestartPlayer - Spawn the player next to his living coop buddy instead of a PlayerStart
*/
void AVCoopGameMode::RestartPlayer(class AController* NewPlayer)
{
	/* Fallback to PlayerStart picking if team spawning is disabled or we're trying to spawn a bot. */
	if (!bSpawnAtTeamPlayer || (NewPlayer->PlayerState && NewPlayer->PlayerState->bIsABot))
	{
		Super::RestartPlayer(NewPlayer);
		return;
	}

	/* Look for a live player to spawn next to */
	FVector SpawnOrigin = FVector::ZeroVector;
	FRotator StartRotation = FRotator::ZeroRotator;
	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; It++)
	{
		AVCharacter* MyCharacter = Cast<AVCharacter>(*It);
		if (MyCharacter && MyCharacter->IsAlive())
		{
			/* Get the origin of the first player we can find */
			SpawnOrigin = MyCharacter->GetActorLocation();
			StartRotation = MyCharacter->GetActorRotation();
			break;
		}
	}

	/* No player is alive (yet) - spawn using one of the PlayerStarts */
	if (SpawnOrigin == FVector::ZeroVector)
	{
		Super::RestartPlayer(NewPlayer);
		return;
	}

	/* Get a point on the nav mesh near the other player */
	FVector StartLocation = UNavigationSystem::GetRandomPointInNavigableRadius(NewPlayer, SpawnOrigin, 250.0f);

	// Try to create a pawn to use of the default class for this player
	if (NewPlayer->GetPawn() == nullptr && GetDefaultPawnClassForController(NewPlayer) != nullptr)
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Instigator = Instigator;
		APawn* ResultPawn = GetWorld()->SpawnActor<APawn>(GetDefaultPawnClassForController(NewPlayer), StartLocation, StartRotation, SpawnInfo);
		if (ResultPawn == nullptr)
		{
			UE_LOG(LogGameMode, Warning, TEXT("Couldn't spawn Pawn of type %s at %s"), *GetNameSafe(DefaultPawnClass), &StartLocation);
		}
		NewPlayer->SetPawn(ResultPawn);
	}

	if (NewPlayer->GetPawn() == nullptr)
	{
		NewPlayer->FailedToSpawnPawn();
	}
	else
	{
		NewPlayer->Possess(NewPlayer->GetPawn());

		// If the Pawn is destroyed as part of possession we have to abort
		if (NewPlayer->GetPawn() == nullptr)
		{
			NewPlayer->FailedToSpawnPawn();
		}
		else
		{
			// Set initial control rotation to player start's rotation
			NewPlayer->ClientSetRotation(NewPlayer->GetPawn()->GetActorRotation(), true);

			FRotator NewControllerRot = StartRotation;
			NewControllerRot.Roll = 0.f;
			NewPlayer->SetControlRotation(NewControllerRot);

			SetPlayerDefaults(NewPlayer->GetPawn());
		}
	}
}


void AVCoopGameMode::OnNightEnded()
{
	/* Respawn spectating players that died during the night */
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		/* Look for all players that are spectating */
		AVPlayerController* MyController = Cast<AVPlayerController>(*It);
		if (MyController)
		{
			if (MyController->PlayerState->bIsSpectator)
			{
				RestartPlayer(MyController);
				MyController->HUDStateChanged(EHUDState::Playing);
			}
			else
			{
				/* Player still alive, award him some points */
				AVCharacter* MyPawn = Cast<AVCharacter>(MyController->GetPawn());
				if (MyPawn && MyPawn->IsAlive())
				{
					AVPlayerState* PS = Cast<AVPlayerState>(MyController->PlayerState);
					if (PS)
					{
						PS->ScorePoints(ScoreNightSurvived);
					}
				}
			}
		}
	}
}


void AVCoopGameMode::Killed(AController* Killer, AController* VictimPlayer, APawn* VictimPawn, const UDamageType* DamageType)
{
	AVPlayerState* KillerPS = Killer ? Cast<AVPlayerState>(Killer->PlayerState) : NULL;
	AVPlayerState* VictimPS = VictimPlayer ? Cast<AVPlayerState>(VictimPlayer->PlayerState) : NULL;

	if (KillerPS && KillerPS != VictimPS && !KillerPS->bIsABot)
	{
		KillerPS->AddKill();
		KillerPS->ScorePoints(10);
	}

	if (VictimPS && !VictimPS->bIsABot)
	{
		VictimPS->AddDeath();
	}

	/* End match is all players died */
	CheckMatchEnd();
}


void AVCoopGameMode::CheckMatchEnd()
{
	bool bHasAlivePlayer = false;
	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; It++)
	{
		AVCharacter* MyPawn = Cast<AVCharacter>(*It);
		if (MyPawn && MyPawn->IsAlive())
		{
			AVPlayerState* PS = Cast<AVPlayerState>(MyPawn->PlayerState);
			if (PS)
			{
				if (!PS->bIsABot)
				{
					/* Found one player that is still alive, game will continue */
					bHasAlivePlayer = true;
					break;
				}
			}
		}
	}

	/* End game is all players died */
	if (!bHasAlivePlayer)
	{
		FinishMatch();
	}
}


void AVCoopGameMode::FinishMatch()
{
	AVGameState* const MyGameState = Cast<AVGameState>(GameState);
	if (IsMatchInProgress())
	{
		EndMatch();

		/* Stop spawning bots */
		GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawns);

		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
		{
			AVPlayerController* MyController = Cast<AVPlayerController>(*It);
			if (MyController)
			{
				MyController->HUDStateChanged(EHUDState::MatchEnd);
			}
		}
	}
}

// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "VIM.h"
#include "VGameMode.h"
#include "VPlayerController.h"
#include "VPlayerState.h"
#include "VGameState.h"
#include "VCharacter.h"
#include "VHUD.h"
#include "VBaseCharacter.h"
#include "VSpectatorPawn.h"
#include "VAIController.h"
#include "VAICharacter.h"
#include "VPlayerStart.h"
#include "VMutator.h"


AVGameMode::AVGameMode(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	/* Assign the class types used by this gamemode */
	PlayerControllerClass = AVCharacter::StaticClass();
	PlayerStateClass = AVPlayerState::StaticClass();
	GameStateClass = AVGameState::StaticClass();
	SpectatorClass = AVSpectatorPawn::StaticClass();

	bAllowFriendlyFireDamage = false;
	bSpawnEnemiesAtNight = true;

	/* Start the game at 16:00 */
	TimeOfDayStart = 16 * 60;
	BotSpawnInterval = 5.0f;

	/* Default team is 1 for players and 0 for enemies */
	PlayerTeamNum = 1;
}


void AVGameMode::InitGameState()
{
	Super::InitGameState();

	AVGameState* MyGameState = Cast<AVGameState>(GameState);
	if (MyGameState)
	{
		MyGameState->ElapsedGameMinutes = TimeOfDayStart;
	}
}


void AVGameMode::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	/* Set timer to run every second */
	GetWorldTimerManager().SetTimer(TimerHandle_DefaultTimer, this, &AVGameMode::DefaultTimer, GetWorldSettings()->GetEffectiveTimeDilation(), true);
}


void AVGameMode::StartMatch()
{
	if (!HasMatchStarted())
	{
		//enable to spawn bots at random
		/* Spawn a new bot every 5 seconds (bothandler will opt-out based on his own rules for example to only spawn during night time) */
		//GetWorldTimerManager().SetTimer(TimerHandle_BotSpawns, this, &AVGameMode::SpawnBotHandler, BotSpawnInterval, true);
	}

	Super::StartMatch();
}


void AVGameMode::DefaultTimer()
{
	/* Immediately start the match while playing in editor */
	//if (GetWorld()->IsPlayInEditor())
	{
		if (GetMatchState() == MatchState::WaitingToStart)
		{
			StartMatch();
		}
	}

	/* Only increment time of day while game is active */
	if (IsMatchInProgress())
	{
		AVGameState* MyGameState = Cast<AVGameState>(GameState);
		if (MyGameState)
		{
			/* Increment our time of day */
			MyGameState->ElapsedGameMinutes += MyGameState->GetTimeOfDayIncrement();

			/* Determine our state */
			MyGameState->GetAndUpdateIsNight();

			/* Trigger events when night starts or ends */
			bool CurrentIsNight = MyGameState->GetIsNight();
			if (CurrentIsNight != LastIsNight)
			{
				AVGameState* MyGameState = Cast<AVGameState>(GameState);
				if (MyGameState)
				{
					EHUDMessage MessageID = CurrentIsNight ? EHUDMessage::Game_SurviveStart : EHUDMessage::Game_SurviveEnded;
					
				}

				/* The night just ended, respawn all dead players */
				if (!CurrentIsNight)
				{
					OnNightEnded();
				}

				/* Update bot states */
				if (CurrentIsNight)
				{
					WakeAllBots();
				}
				else
				{
					PassifyAllBots();
				}
			}

			LastIsNight = MyGameState->bIsNight;
		}
	}
}


bool AVGameMode::CanDealDamage(class AVPlayerState* DamageCauser, class AVPlayerState* DamagedPlayer) const
{
	if (bAllowFriendlyFireDamage)
		return true;

	/* Allow damage to self */
	if (DamagedPlayer == DamageCauser)
		return true;

	// Compare Team Numbers
	return DamageCauser && DamagedPlayer && (DamageCauser->GetTeamNumber() != DamagedPlayer->GetTeamNumber());
}


FString AVGameMode::InitNewPlayer(class APlayerController* NewPlayerController, const TSharedPtr<const FUniqueNetId>& UniqueId, const FString& Options, const FString& Portal)
{
	FString Result = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

	AVPlayerState* NewPlayerState = Cast<AVPlayerState>(NewPlayerController->PlayerState);
	if (NewPlayerState)
	{
		NewPlayerState->SetTeamNumber(PlayerTeamNum);
	}

	return Result;
}


float AVGameMode::ModifyDamage(float Damage, AActor* DamagedActor, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const
{
	float ActualDamage = Damage;

	AVBaseCharacter* DamagedPawn = Cast<AVBaseCharacter>(DamagedActor);
	if (DamagedPawn && EventInstigator)
	{
		AVPlayerState* DamagedPlayerState = Cast<AVPlayerState>(DamagedPawn->PlayerState);
		AVPlayerState* InstigatorPlayerState = Cast<AVPlayerState>(EventInstigator->PlayerState);

		// Check for friendly fire
		if (!CanDealDamage(InstigatorPlayerState, DamagedPlayerState))
		{
			ActualDamage = 0.f;
		}
	}

	return ActualDamage;
}


bool AVGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	/* Always pick a random location */
	return false;
}


AActor* AVGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<APlayerStart*> PreferredSpawns;
	TArray<APlayerStart*> FallbackSpawns;

	/* Get all playerstart objects in level */
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);

	/* Split the player starts into two arrays for preferred and fallback spawns */
	for (int32 i = 0; i < PlayerStarts.Num(); i++)
	{
		APlayerStart* TestStart = Cast<APlayerStart>(PlayerStarts[i]);

		if (TestStart && IsSpawnpointAllowed(TestStart, Player))
		{
			if (IsSpawnpointPreferred(TestStart, Player))
			{
				PreferredSpawns.Add(TestStart);
			}
			else
			{
				FallbackSpawns.Add(TestStart);
			}
		}

	}

	/* Pick a random spawnpoint from the filtered spawn points */
	APlayerStart* BestStart = nullptr;
	if (PreferredSpawns.Num() > 0)
	{
		BestStart = PreferredSpawns[FMath::RandHelper(PreferredSpawns.Num())];
	}
	else if (FallbackSpawns.Num() > 0)
	{
		BestStart = FallbackSpawns[FMath::RandHelper(FallbackSpawns.Num())];
	}

	/* If we failed to find any (so BestStart is nullptr) fall back to the base code */
	return BestStart ? BestStart : Super::ChoosePlayerStart_Implementation(Player);
}


bool AVGameMode::IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Controller)
{
	if (Controller == nullptr || Controller->PlayerState == nullptr)
		return true;

	/* Check for extended playerstart class */
	AVPlayerStart* MyPlayerStart = Cast<AVPlayerStart>(SpawnPoint);
	if (MyPlayerStart)
	{
		return MyPlayerStart->GetIsPlayerOnly() && !Controller->PlayerState->bIsABot;
	}

	/* Cast failed, Anyone can spawn at the base playerstart class */
	return true;
}


bool AVGameMode::IsSpawnpointPreferred(APlayerStart* SpawnPoint, AController* Controller)
{
	if (SpawnPoint)
	{
		/* Iterate all pawns to check for collision overlaps with the spawn point */
		const FVector SpawnLocation = SpawnPoint->GetActorLocation();
		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; It++)
		{
			ACharacter* OtherPawn = Cast<ACharacter>(*It);
			if (OtherPawn)
			{
				const float CombinedHeight = (SpawnPoint->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + OtherPawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()) * 2.0f;
				const float CombinedWidth = SpawnPoint->GetCapsuleComponent()->GetScaledCapsuleRadius() + OtherPawn->GetCapsuleComponent()->GetScaledCapsuleRadius();
				const FVector OtherLocation = OtherPawn->GetActorLocation();

				// Check if player overlaps the playerstart
				if (FMath::Abs(SpawnLocation.Z - OtherLocation.Z) < CombinedHeight && (SpawnLocation - OtherLocation).Size2D() < CombinedWidth)
				{
					return false;
				}
			}
		}

		/* Check if spawnpoint is exclusive to players */
		AVPlayerStart* MyPlayerStart = Cast<AVPlayerStart>(SpawnPoint);
		if (MyPlayerStart)
		{
			return MyPlayerStart->GetIsPlayerOnly() && !Controller->PlayerState->bIsABot;
		}
	}

	return false;
}


void AVGameMode::SpawnNewBot()
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AVAIController* AIC = GetWorld()->SpawnActor<AVAIController>(SpawnInfo);
	RestartPlayer(AIC);
}

/* Used by RestartPlayer() to determine the pawn to create and possess when a bot or player spawns */
UClass* AVGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (Cast<AVAIController>(InController))
	{
		return BotPawnClass;
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}


bool AVGameMode::CanSpectate_Implementation(APlayerController* Viewer, APlayerState* ViewTarget)
{
	/* Don't allow spectating of other non-player bots */
	return (ViewTarget && !ViewTarget->bIsABot);
}


void AVGameMode::PassifyAllBots()
{
	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; It++)
	{
		AVAICharacter* AIPawn = Cast<AVAICharacter>(*It);
		if (AIPawn)
		{
			AIPawn->SetBotType(EBotBehaviorType::Passive);
		}
	}
}


void AVGameMode::WakeAllBots()
{
	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; It++)
	{
		AVAICharacter* AIPawn = Cast<AVAICharacter>(*It);
		if (AIPawn)
		{
			AIPawn->SetBotType(EBotBehaviorType::Patrolling);
		}
	}
}


void AVGameMode::SpawnBotHandler()
{
	if (!bSpawnEnemiesAtNight)
		return;

	AVGameState* MyGameState = Cast<AVGameState>(GameState);
	if (MyGameState)
	{
		/* Only spawn bots during night time */
		if (MyGameState->GetIsNight())
		{
			/* This could be any dynamic number based on difficulty (eg. increasing after having survived a few nights) */
			const int32 MaxPawns = 10;

			/* Check number of available pawns (players included) */
			if (GetWorld()->GetNumPawns() < MaxPawns)
			{
				SpawnNewBot();
			}
		}
	}
}


void AVGameMode::OnNightEnded()
{
	// Do nothing (can be used to apply score or trigger other time of day events)
}

void AVGameMode::Killed(AController* Killer, AController* VictimPlayer, APawn* VictimPawn, const UDamageType* DamageType)
{
	// Do nothing (can we used to apply score or keep track of kill count)
}


void AVGameMode::SetPlayerDefaults(APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);

	SpawnDefaultInventory(PlayerPawn);
}


void AVGameMode::SpawnDefaultInventory(APawn* PlayerPawn)
{
	AVCharacter* MyPawn = Cast<AVCharacter>(PlayerPawn);
	if (MyPawn)
	{
		for (int32 i = 0; i < DefaultInventoryClasses.Num(); i++)
		{
			if (DefaultInventoryClasses[i])
			{
				FActorSpawnParameters SpawnInfo;
				SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				AVWeapon* NewWeapon = GetWorld()->SpawnActor<AVWeapon>(DefaultInventoryClasses[i], SpawnInfo);

				MyPawn->AddWeapon(NewWeapon);
			}
		}
	}
}


/************************************************************************/
/* Modding & Mutators                                                   */
/************************************************************************/


void AVGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	// HACK: workaround to inject CheckRelevance() into the BeginPlay sequence
	UFunction* Func = AActor::GetClass()->FindFunctionByName(FName(TEXT("ReceiveBeginPlay")));
	Func->FunctionFlags |= FUNC_Native;
	Func->SetNativeFunc((Native)&AVGameMode::BeginPlayMutatorHack);

	/* Spawn all mutators. */
	for (int32 i = 0; i < MutatorClasses.Num(); i++)
	{
		AddMutator(MutatorClasses[i]);
	}

	if (BaseMutator)
	{
		BaseMutator->InitGame(MapName, Options, ErrorMessage);
	}

	Super::InitGame(MapName, Options, ErrorMessage);
}


bool AVGameMode::CheckRelevance_Implementation(AActor* Other)
{
	/* Execute the first in the mutator chain */
	if (BaseMutator)
	{
		return BaseMutator->CheckRelevance(Other);
	}

	return true;
}


void AVGameMode::BeginPlayMutatorHack(FFrame& Stack, RESULT_DECL)
{
	P_FINISH;

	// WARNING: This function is called by every Actor in the level during his BeginPlay sequence. Meaning:  'this' is actually an AActor! Only do AActor things!
	if (!IsA(ALevelScriptActor::StaticClass()) && !IsA(AVMutator::StaticClass()) &&
		(RootComponent == NULL || RootComponent->Mobility != EComponentMobility::Static || (!IsA(AStaticMeshActor::StaticClass()) && !IsA(ALight::StaticClass()))))
	{
		AVGameMode* Game = GetWorld()->GetAuthGameMode<AVGameMode>();
		// a few type checks being AFTER the CheckRelevance() call is intentional; want mutators to be able to modify, but not outright destroy
		if (Game != NULL && Game != this && !Game->CheckRelevance((AActor*)this) && !IsA(APlayerController::StaticClass()))
		{
			/* Actors are destroyed if they fail the relevance checks (which moves through the gamemode specific check AND the chain of mutators) */
			Destroy();
		}
	}
}


void AVGameMode::AddMutator(TSubclassOf<AVMutator> MutClass)
{
	AVMutator* NewMut = GetWorld()->SpawnActor<AVMutator>(MutClass);
	if (NewMut)
	{
		if (BaseMutator == nullptr)
		{
			BaseMutator = NewMut;
		}
		else
		{
			// Add as child in chain
			BaseMutator->NextMutator = NewMut;
		}
	}
}

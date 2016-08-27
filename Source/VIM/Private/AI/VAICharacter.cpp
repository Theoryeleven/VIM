// copyright The Perfect Game Company 2016

#include "VIM.h"
#include "VAICharacter.h"
#include "VAIController.h"
#include "VCharacter.h"
#include "VBaseCharacter.h"
#include "VWayPoint.h"
#include "VPlayerState.h"

/* AI Include */
#include "Perception/PawnSensingComponent.h"

// Sets default values
AVAICharacter::AVAICharacter(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	/* Note: We assign the Controller class in the Blueprint extension of this class
	Because the AIController is a blueprint in content and it's better to avoid content references in code.  */
	/*AIControllerClass = AVAIController::StaticClass();*/

	/* Our sensing component to detect players by visibility and noise checks. */
	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));
	PawnSensingComp->SetPeripheralVisionAngle(60.0f);
	PawnSensingComp->SightRadius = 2000;
	PawnSensingComp->HearingThreshold = 600;
	PawnSensingComp->LOSHearingThreshold = 1200;

	/* Ignore this channel or it will absorb the trace impacts instead of the skeletal mesh */
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f, false);
	GetCapsuleComponent()->SetCapsuleRadius(42.0f);

	/* These values are matched up to the CapsuleComponent above and are used to find navigation paths */
	GetMovementComponent()->NavAgentProps.AgentRadius = 42;
	GetMovementComponent()->NavAgentProps.AgentHeight = 192;

	MeleeCollisionComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("MeleeCollision"));
	MeleeCollisionComp->SetRelativeLocation(FVector(45, 0, 25));
	MeleeCollisionComp->SetCapsuleHalfHeight(60);
	MeleeCollisionComp->SetCapsuleRadius(35, false);
	MeleeCollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeleeCollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	MeleeCollisionComp->SetupAttachment(GetCapsuleComponent());

	AudioLoopComp = CreateDefaultSubobject<UAudioComponent>(TEXT("ZombieLoopedSoundComp"));
	AudioLoopComp->bAutoActivate = false;
	AudioLoopComp->bAutoDestroy = false;
	AudioLoopComp->SetupAttachment(RootComponent);

	Health = 100;
	MeleeDamage = 24.0f;
	MeleeStrikeCooldown = 1.0f;
	SprintingSpeedModifier = 3.0f;

	/* By default we will not let the AI patrol, we can override this value per-instance. */
	BotType = EBotBehaviorType::Passive;
	SenseTimeOut = 2.5f;

	/* Note: Visual Setup is done in the AICharacter Blueprint file */
}


void AVAICharacter::BeginPlay()
{
	Super::BeginPlay();

	/* This is the earliest moment we can bind our delegates to the component */
	if (PawnSensingComp)
	{
		PawnSensingComp->OnSeePawn.AddDynamic(this, &AVAICharacter::OnSeePlayer);
		PawnSensingComp->OnHearNoise.AddDynamic(this, &AVAICharacter::OnHearNoise);
	}
	if (MeleeCollisionComp)
	{
		MeleeCollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AVAICharacter::OnMeleeCompBeginOverlap);
	}

	BroadcastUpdateAudioLoop(bSensedTarget);

	/* Assign a basic name to identify the bots in the HUD. */
	AVPlayerState* PS = Cast<AVPlayerState>(PlayerState);
	if (PS)
	{
		PS->SetPlayerName("Bot");
		PS->bIsABot = true;
	}
}


void AVAICharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	/* Check if the last time we sensed a player is beyond the time out value to prevent bot from endlessly following a player. */
	if (bSensedTarget && (GetWorld()->TimeSeconds - LastSeenTime) > SenseTimeOut
		&& (GetWorld()->TimeSeconds - LastHeardTime) > SenseTimeOut)
	{
		AVAIController* AIController = Cast<AVAIController>(GetController());
		if (AIController)
		{
			bSensedTarget = false;
			/* Reset */
			AIController->SetTargetEnemy(nullptr);

			/* Stop playing the hunting sound */
			BroadcastUpdateAudioLoop(false);
		}
	}
}


void AVAICharacter::OnSeePlayer(APawn* Pawn)
{
	if (!IsAlive())
	{
		return;
	}

	if (!bSensedTarget)
	{
		BroadcastUpdateAudioLoop(true);
	}

	/* Keep track of the time the player was last sensed in order to clear the target */
	LastSeenTime = GetWorld()->GetTimeSeconds();
	bSensedTarget = true;

	AVAIController* AIController = Cast<AVAIController>(GetController());
	AVBaseCharacter* SensedPawn = Cast<AVBaseCharacter>(Pawn);
	if (AIController && SensedPawn->IsAlive())
	{
		AIController->SetTargetEnemy(SensedPawn);
	}
}


void AVAICharacter::OnHearNoise(APawn* PawnInstigator, const FVector& Location, float Volume)
{
	if (!IsAlive())
	{
		return;
	}

	if (!bSensedTarget)
	{
		BroadcastUpdateAudioLoop(true);
	}

	bSensedTarget = true;
	LastHeardTime = GetWorld()->GetTimeSeconds();

	AVAIController* AIController = Cast<AVAIController>(GetController());
	if (AIController)
	{
		AIController->SetTargetEnemy(PawnInstigator);
	}
}


void AVAICharacter::PerformMeleeStrike(AActor* HitActor)
{
	if (LastMeleeAttackTime > GetWorld()->GetTimeSeconds() - MeleeStrikeCooldown)
	{
		/* Set timer to start attacking as soon as the cooldown elapses. */
		if (!TimerHandle_MeleeAttack.IsValid())
		{
			// TODO: Set Timer
		}

		/* Attacked before cooldown expired */
		return;
	}

	if (HitActor && HitActor != this && IsAlive())
	{
		ACharacter* OtherPawn = Cast<ACharacter>(HitActor);
		if (OtherPawn)
		{
			AVPlayerState* MyPS = Cast<AVPlayerState>(PlayerState);
			AVPlayerState* OtherPS = Cast<AVPlayerState>(OtherPawn->PlayerState);

			if (MyPS && OtherPS)
			{
				if (MyPS->GetTeamNumber() == OtherPS->GetTeamNumber())
				{
					/* Do not attack other zombies. */
					return;
				}

				/* Set to prevent a zombie to attack multiple times in a very short time */
				LastMeleeAttackTime = GetWorld()->GetTimeSeconds();

				FPointDamageEvent DmgEvent;
				DmgEvent.DamageTypeClass = PunchDamageType;
				DmgEvent.Damage = MeleeDamage;

				HitActor->TakeDamage(DmgEvent.Damage, DmgEvent, GetController(), this);

				SimulateMeleeStrike();
			}
		}
	}
}


void AVAICharacter::SetBotType(EBotBehaviorType NewType)
{
	BotType = NewType;

	AVAIController* AIController = Cast<AVAIController>(GetController());
	if (AIController)
	{
		AIController->SetBlackboardBotType(NewType);
	}

	BroadcastUpdateAudioLoop(bSensedTarget);
}


UAudioComponent* AVAICharacter::PlayCharacterSound(USoundCue* CueToPlay)
{
	if (CueToPlay)
	{
		return UGameplayStatics::SpawnSoundAttached(CueToPlay, RootComponent, NAME_None, FVector::ZeroVector, EAttachLocation::SnapToTarget, true);
	}

	return nullptr;
}



void AVAICharacter::SimulateMeleeStrike_Implementation()
{
	PlayAnimMontage(MeleeAnimMontage);
	PlayCharacterSound(SoundAttackMelee);
}


void AVAICharacter::OnMeleeCompBeginOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	/* Stop any running attack timers */
	TimerHandle_MeleeAttack.Invalidate();

	PerformMeleeStrike(OtherActor);

	/* Set re-trigger timer to re-check overlapping pawns at melee attack rate interval */
	GetWorldTimerManager().SetTimer(TimerHandle_MeleeAttack, this, &AVAICharacter::OnRetriggerMeleeStrike, MeleeStrikeCooldown, true);
}


void AVAICharacter::OnRetriggerMeleeStrike()
{
	/* Apply damage to a single random pawn in range. */
	TArray<AActor*> Overlaps;
	MeleeCollisionComp->GetOverlappingActors(Overlaps, AVBaseCharacter::StaticClass());
	for (int32 i = 0; i < Overlaps.Num(); i++)
	{
		AVBaseCharacter* OverlappingPawn = Cast<AVBaseCharacter>(Overlaps[i]);
		if (OverlappingPawn)
		{
			PerformMeleeStrike(OverlappingPawn);
			//break; /* Uncomment to only attack one pawn maximum */
		}
	}

	/* No pawns in range, cancel the retrigger timer */
	if (Overlaps.Num() == 0)
	{
		TimerHandle_MeleeAttack.Invalidate();
	}
}
void AVAICharacter::BroadcastUpdateAudioLoop_Implementation(bool bNewSensedTarget)
{
	/* Start playing the hunting sound and the "noticed player" sound if the state is about to change */
	if (bNewSensedTarget && !bSensedTarget)
	{
		PlayCharacterSound(SoundPlayerNoticed);

		AudioLoopComp->SetSound(SoundHunting);
		AudioLoopComp->Play();
	}
	else
	{
		if (BotType == EBotBehaviorType::Patrolling)
		{
			AudioLoopComp->SetSound(SoundWandering);
			AudioLoopComp->Play();
		}
		else
		{
			AudioLoopComp->SetSound(SoundIdle);
			AudioLoopComp->Play();
		}
	}
}
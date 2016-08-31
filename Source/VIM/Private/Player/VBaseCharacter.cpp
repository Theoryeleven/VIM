// copyright The Perfect Game Company 2016

#include "VIM.h"
#include "VDamageType.h"
#include "VBaseCharacter.h"
#include "VGameMode.h"


// Sets default values
AVBaseCharacter::AVBaseCharacter(const class FObjectInitializer& ObjectInitializer)
/* Override the movement class from the base class to our own to support multiple speeds (sprinting) */
	//: Super(ObjectInitializer.SetDefaultSubobjectClass<UCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	
	Health = 100;

	TargetingSpeedModifier = 0.5f;
	SprintingSpeedModifier = 2.0f;

	/* Noise emitter for both players and enemies. This keeps track of MakeNoise data and is used by the pawn sensing component in our SZombieCharacter class */
	NoiseEmitterComp = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("NoiseEmitterComp"));

	/* Don't collide with camera checks to keep 3rd person camera at position when zombies or other players are standing behind us */
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

float AVBaseCharacter::GetHealth() const
{
	return Health;
}
float AVBaseCharacter::GetHealthPercentage() const
{
	
	return (Health / GetMaxHealth());
}
float AVBaseCharacter::GetMaxHealth() const
{
	// Retrieve the default value of the health property that is assigned on instantiation.
	return GetClass()->GetDefaultObject<AVBaseCharacter>()->Health;
}

float AVBaseCharacter::GetShields() const
{
	return Shields;
}
float AVBaseCharacter::GetMaxShields() const
{
	return GetClass()->GetDefaultObject<AVBaseCharacter>()->Shields;
}

float AVBaseCharacter::GetShieldPercentage() const
{
	return (Shields / MaxShields);
}

bool AVBaseCharacter::IsAlive() const
{
	return Health > 0;
}
float AVBaseCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser)
{
	if (Health <= 0.f)
	{
		return 0.f;
	}
	/* Modify based based on game type rules */
	AVGameMode* MyGameMode = Cast<AVGameMode>(GetWorld()->GetAuthGameMode());
	Damage = MyGameMode ? MyGameMode->ModifyDamage(Damage, this, DamageEvent, EventInstigator, DamageCauser) : Damage;
	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage > 0.f)
	{
		if (Shields > 0)
		{
			Shields -= ActualDamage;
		}
		else
		{
			Health -= ActualDamage;
			if (Health <= 0)
			{
				bool bCanDie = true;

				/* Check the damage type, always allow dying if the cast fails, otherwise check the property if player can die from damage type */
				if (DamageEvent.DamageTypeClass)
				{
					UVDamageType* DmgType = Cast<UVDamageType>(DamageEvent.DamageTypeClass->GetDefaultObject());
					bCanDie = (DmgType == nullptr || (DmgType && DmgType->GetCanDieFrom()));
				}

				if (bCanDie)
				{
					Die(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
				}
				else
				{
					/* Player cannot die from this damage type, set hit points to 1.0 */
					Health = 1.0f;
				}
			}
		}
	}

	return ActualDamage;
}
bool AVBaseCharacter::CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser) const
{
	/* Check if character is already dying, destroyed or if we have authority */
	if (bIsDying ||
		IsPendingKill() ||
		GetWorld()->GetAuthGameMode() == NULL )
		
	{
		return false;
	}

	return true;
}


void AVBaseCharacter::FellOutOfWorld(const class UDamageType& DmgType)
{
	Die(Health, FDamageEvent(DmgType.GetClass()), NULL, NULL);
}


bool AVBaseCharacter::Die(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser)
{
	if (!CanDie(KillingDamage, DamageEvent, Killer, DamageCauser))
	{
		return false;
	}

	Health = FMath::Min(0.0f, Health);

	/* Fallback to default DamageType if none is specified */
	UDamageType const* const DamageType = DamageEvent.DamageTypeClass ? DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>() : GetDefault<UDamageType>();
	Killer = GetDamageInstigator(Killer, *DamageType);

	/* Notify the gamemode we got killed for scoring and game over state */
	//AController* KilledPlayer = Controller ? Controller : Cast<AController>(GetOwner());
	//GetWorld()->GetAuthGameMode<AVGameMode>()->Killed(Killer, KilledPlayer, this, DamageType);

	OnDeath(KillingDamage, DamageEvent, Killer ? Killer->GetPawn() : NULL, DamageCauser);
	return true;
}


void AVBaseCharacter::OnDeath(float KillingDamage, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser)
{
	if (bIsDying)
	{
		return;
	}

	bReplicateMovement = false;
	bTearOff = true;
	bIsDying = true;

	DetachFromControllerPendingDestroy();

	/* Disable all collision on capsule */
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);

	USkeletalMeshComponent* Mesh3P = GetMesh();
	if (Mesh3P)
	{
		Mesh3P->SetCollisionProfileName(TEXT("Ragdoll"));
	}
	SetActorEnableCollision(true);

	SetRagdollPhysics();

	/* Apply physics impulse on the bone of the enemy skeleton mesh we hit (ray-trace damage only) */
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		FPointDamageEvent PointDmg = *((FPointDamageEvent*)(&DamageEvent));
		{
			// TODO: Use DamageTypeClass->DamageImpulse
			Mesh3P->AddImpulseAtLocation(PointDmg.ShotDirection * 12000, PointDmg.HitInfo.ImpactPoint, PointDmg.HitInfo.BoneName);
		}
	}
	if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
	{
		FRadialDamageEvent RadialDmg = *((FRadialDamageEvent const*)(&DamageEvent));
		{
			Mesh3P->AddRadialImpulse(RadialDmg.Origin, RadialDmg.Params.GetMaxRadius(), 100000 /*RadialDmg.DamageTypeClass->DamageImpulse*/, ERadialImpulseFalloff::RIF_Linear);
		}
	}
}


void AVBaseCharacter::SetRagdollPhysics()
{
	bool bInRagdoll = false;
	USkeletalMeshComponent* Mesh3P = GetMesh();

	if (IsPendingKill())
	{
		bInRagdoll = false;
	}
	else if (!Mesh3P || !Mesh3P->GetPhysicsAsset())
	{
		bInRagdoll = false;
	}
	else
	{
		Mesh3P->SetAllBodiesSimulatePhysics(true);
		Mesh3P->SetSimulatePhysics(true);
		Mesh3P->WakeAllRigidBodies();
		Mesh3P->bBlendPhysics = true;

		bInRagdoll = true;
	}

	UCharacterMovementComponent* CharacterComp = Cast<UCharacterMovementComponent>(GetMovementComponent());
	if (CharacterComp)
	{
		CharacterComp->StopMovementImmediately();
		CharacterComp->DisableMovement();
		CharacterComp->SetComponentTickEnabled(false);
	}

	if (!bInRagdoll)
	{
		// Immediately hide the pawn
		TurnOff();
		SetActorHiddenInGame(true);
		SetLifeSpan(1.0f);
	}
	else
	{
		SetLifeSpan(10.0f);
	}
}

void AVBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Value is already updated locally, skip in replication step
	

	// Replicate to every client, no special condition required
	DOREPLIFETIME(AVBaseCharacter, Health);
	DOREPLIFETIME(AVBaseCharacter, Shields);
	
}
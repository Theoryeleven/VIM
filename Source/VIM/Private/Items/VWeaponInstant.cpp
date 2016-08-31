// Fill out your copyright notice in the Description page of Project Settings.

#include "VIM.h"
#include "VWeaponInstant.h"
#include "VImpactEffect.h"
#include "VCharacter.h"
#include "VPlayerController.h"
#include "VDamageType.h"


AVWeaponInstant::AVWeaponInstant(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	HitDamage = 26;
	WeaponRange = 15000;
	Spread = 0.0f;
	AllowedViewDotHitDir = -1.0f;
	ClientSideHitLeeway = 200.0f;
	MinimumProjectileSpawnDistance = 800;
	TracerRoundInterval = 3;
	TraceParams = FCollisionQueryParams(FName(TEXT("HitscanTrace")), true, this);
	//TraceParams.TraceTag = FName("HitscanTrace");
}

/* old Fire Method
void AVWeaponInstant::FireWeapon()
{
	
	
	const FVector AimDir = GetAdjustedAim();// direction of character
	const FVector CameraPos = GetCameraDamageStartLocation(AimDir);// looking down barrel
	const FVector EndPos = CameraPos + (AimDir * WeaponRange);// shot looking down barrel to range
	/* Check for impact by tracing from the camera position 
	FHitResult Impact = WeaponTrace(CameraPos, EndPos);
	const FVector MuzzleOrigin = GetMuzzleLocation();// location of muzzel socket
	FVector AdjustedAimDir = AimDir;
	if (Impact.bBlockingHit)
	{
		/* Adjust the shoot direction to hit at the cross hair. 
		AdjustedAimDir = (Impact.ImpactPoint - MuzzleOrigin).GetSafeNormal();

		/* Re-trace with the new aim direction coming out of the weapon muzzle 
		Impact = WeaponTrace(MuzzleOrigin, MuzzleOrigin + (AdjustedAimDir * WeaponRange));
	}
	else
	{
		/* Use the maximum distance as the adjust direction 
		Impact.ImpactPoint = FVector_NetQuantize(EndPos);
	}

	ProcessInstantHit(Impact, MuzzleOrigin, AdjustedAimDir);
}*/

void AVWeaponInstant::FireWeapon()
{
	FHitResult Hit(ForceInit);
	FVector Start = GetMuzzleLocation();
	FVector End = Start + CalcSpread() * WeaponRange;

	GetWorld()->LineTraceSingleByChannel(Hit, Start, End, COLLISION_WEAPON, TraceParams);

	UE_LOG(LogTemp, Warning, TEXT("Fire!"));

	CurrentAmmo--;
	ProcessInstantHit(Hit, Start, End);

	if (Hit.GetActor()) {
		UE_LOG(LogTemp, Warning, TEXT("Hit! %s"), *Hit.GetActor()->GetName());
		DealDamage(Hit);
		SpawnImpactEffects(Hit);
	}
}

FVector AVWeaponInstant::CalcSpread() {
	if (GetOwner()) {
		FVector Direction = GetOwner()->GetActorRotation().Vector();
		float Angle = atan(Spread / 10000);
		return FMath::VRandCone(Direction, Angle);
	}
	else {
		FVector Direction = GetActorRotation().Vector();
		float Angle = atan(Spread / 10000);
		return FMath::VRandCone(Direction, Angle);
	}
}


bool AVWeaponInstant::ShouldDealDamage(AActor* TestActor) const
{
	// If we are an actor on the server, or the local client has authoritative control over actor, we should register damage.
	if (TestActor)
	{
		if (GetNetMode() != NM_Client ||
			TestActor->Role == ROLE_Authority ||
			TestActor->bTearOff)
		{
			return true;
		}
	}

	return false;
}

void AVWeaponInstant::DealDamage(const FHitResult& Hit) {
	if (Hit.GetActor()) {
		float ActualHitDamage = HitDamage;
		/* for when we have physics surfaces in 
		UVDamageType* DmgType = Cast<UVDamageType>(DamageType->GetDefaultObject(true));
		UPhysicalMaterial * PhysMat = Hit.PhysMaterial.Get();
		if (PhysMat && DmgType)
		{
			if (PhysMat->SurfaceType == SURFACE_DEFAULT)
			{
				ActualHitDamage *= DmgType->GetDefaultDamageModifier();
			}
			else if (PhysMat->SurfaceType == SURFACE_BODY)
			{
				ActualHitDamage *= DmgType->GetBodyDamageModifier();
			}
			else if (PhysMat->SurfaceType == SURFACE_ARMOUR)
			{
				ActualHitDamage *= DmgType->GetArmourDamageModifier();
			}
			else if (PhysMat->SurfaceType == SURFACE_SHIELDS)
			{
				ActualHitDamage *= DmgType->GetShieldDamageModifier();
			}
			else if (PhysMat->SurfaceType == SURFACE_INANIMATE)
			{
				ActualHitDamage *= DmgType->GetInanimateDamageModifier();
			}
		}*/
		FVector ShotDir = GetActorLocation() - Hit.ImpactPoint;

		FPointDamageEvent DamageEvent;
		DamageEvent.Damage = ActualHitDamage;
		DamageEvent.HitInfo = Hit;
		DamageEvent.ShotDirection = ShotDir;
		DamageEvent.ShotDirection.Normalize();

		Hit.GetActor()->TakeDamage(ActualHitDamage, DamageEvent, MyPawn->Controller, this);
	}
}
/* old damage method
void AVWeaponInstant::DealDamage(const FHitResult& Impact, const FVector& ShootDir)
{
	float ActualHitDamage = HitDamage;

	// Handle special damage (types are setup in the Physics Asset 
	UVDamageType* DmgType = Cast<UVDamageType>(DamageType->GetDefaultObject());
	UPhysicalMaterial * PhysMat = Impact.PhysMaterial.Get();
	if (PhysMat && DmgType)
	{
		if (PhysMat->SurfaceType == SURFACE_DEFAULT)
		{
			ActualHitDamage *= DmgType->GetDefaultDamageModifier();
		}
		else if (PhysMat->SurfaceType == SURFACE_BODY)
		{
			ActualHitDamage *= DmgType->GetBodyDamageModifier();
		}
		else if (PhysMat->SurfaceType == SURFACE_ARMOUR)
		{
			ActualHitDamage *= DmgType->GetArmourDamageModifier();
		}
		else if (PhysMat->SurfaceType == SURFACE_SHIELDS)
		{
			ActualHitDamage *= DmgType->GetShieldDamageModifier();
		}
		else if (PhysMat->SurfaceType == SURFACE_INANIMATE)
		{
			ActualHitDamage *= DmgType->GetInanimateDamageModifier();
		}
	}

	FPointDamageEvent PointDmg;
	PointDmg.DamageTypeClass = DamageType;
	PointDmg.HitInfo = Impact;
	PointDmg.ShotDirection = ShootDir;
	PointDmg.Damage = ActualHitDamage;

	Impact.GetActor()->TakeDamage(PointDmg.Damage, PointDmg, MyPawn->Controller, this);
}*/


void AVWeaponInstant::ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir)
{
	if (MyPawn && MyPawn->IsLocallyControlled() && GetNetMode() == NM_Client)
	{
		// If we are a client and hit something that is controlled by server
		if (Impact.GetActor() && Impact.GetActor()->GetRemoteRole() == ROLE_Authority)
		{
			// Notify the server of our local hit to validate and apply actual hit damage.
			ServerNotifyHit(Impact, ShootDir);
		}
		else if (Impact.GetActor() == nullptr)
		{
			if (Impact.bBlockingHit)
			{
				ServerNotifyHit(Impact, ShootDir);
			}
			else
			{
				ServerNotifyMiss(ShootDir);
			}
		}
	}

	// Process a confirmed hit.
	ProcessInstantHitConfirmed(Impact, Origin, ShootDir);
}


void AVWeaponInstant::ProcessInstantHitConfirmed(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir)
{
	// Handle damage
	if (ShouldDealDamage(Impact.GetActor()))
	{
		DealDamage(Impact);
	}

	// Play FX on remote clients
	if (Role == ROLE_Authority)
	{
		HitImpactNotify = Impact.ImpactPoint;
	}

	// Play FX locally
	if (GetNetMode() != NM_DedicatedServer)
	{
		SimulateInstantHit(Impact.ImpactPoint);
	}
}


void AVWeaponInstant::SimulateInstantHit(const FVector& ImpactPoint)
{
	const FVector MuzzleOrigin = GetMuzzleLocation();

	/* Adjust direction based on desired crosshair impact point and muzzle location */
	const FVector AimDir = (ImpactPoint - MuzzleOrigin).GetSafeNormal();

	const FVector EndTrace = MuzzleOrigin + (AimDir * WeaponRange);
	const FHitResult Impact = WeaponTrace(MuzzleOrigin, EndTrace);

	if (Impact.bBlockingHit)
	{
		SpawnImpactEffects(Impact);
		SpawnTrailEffects(Impact.ImpactPoint);
	}
	else
	{
		SpawnTrailEffects(EndTrace);
	}
}


bool AVWeaponInstant::ServerNotifyHit_Validate(const FHitResult Impact, FVector_NetQuantizeNormal ShootDir)
{
	return true;
}


void AVWeaponInstant::ServerNotifyHit_Implementation(const FHitResult Impact, FVector_NetQuantizeNormal ShootDir)
{
	// If we have an instigator, calculate the dot between the view and the shot
	if (Instigator && (Impact.GetActor() || Impact.bBlockingHit))
	{
		const FVector Origin = GetMuzzleLocation();
		const FVector ViewDir = (Impact.Location - Origin).GetSafeNormal();

		const float ViewDotHitDir = FVector::DotProduct(Instigator->GetViewRotation().Vector(), ViewDir);
		if (ViewDotHitDir > AllowedViewDotHitDir)
		{
			// TODO: Check for weapon state

			if (Impact.GetActor() == nullptr)
			{
				if (Impact.bBlockingHit)
				{
					ProcessInstantHitConfirmed(Impact, Origin, ShootDir);
				}
			}
			// Assume it told the truth about static things because we don't move and the hit
			// usually doesn't have significant gameplay implications
			else if (Impact.GetActor()->IsRootComponentStatic() || Impact.GetActor()->IsRootComponentStationary())
			{
				ProcessInstantHitConfirmed(Impact, Origin, ShootDir);
			}
			else
			{
				const FBox HitBox = Impact.GetActor()->GetComponentsBoundingBox();

				FVector BoxExtent = 0.5 * (HitBox.Max - HitBox.Min);
				BoxExtent *= ClientSideHitLeeway;

				BoxExtent.X = FMath::Max(20.0f, BoxExtent.X);
				BoxExtent.Y = FMath::Max(20.0f, BoxExtent.Y);
				BoxExtent.Z = FMath::Max(20.0f, BoxExtent.Z);

				const FVector BoxCenter = (HitBox.Min + HitBox.Max) * 0.5;

				// If we are within client tolerance
				if (FMath::Abs(Impact.Location.Z - BoxCenter.Z) < BoxExtent.Z &&
					FMath::Abs(Impact.Location.X - BoxCenter.X) < BoxExtent.X &&
					FMath::Abs(Impact.Location.Y - BoxCenter.Y) < BoxExtent.Y)
				{
					ProcessInstantHitConfirmed(Impact, Origin, ShootDir);
				}
			}
		}
	}

	// TODO: UE_LOG on failures & rejection
}


bool AVWeaponInstant::ServerNotifyMiss_Validate(FVector_NetQuantizeNormal ShootDir)
{
	return true;
}


void AVWeaponInstant::ServerNotifyMiss_Implementation(FVector_NetQuantizeNormal ShootDir)
{
	const FVector Origin = GetMuzzleLocation();
	const FVector EndTrace = Origin + (ShootDir * WeaponRange);

	// Play on remote clients
	HitImpactNotify = EndTrace;

	if (GetNetMode() != NM_DedicatedServer)
	{
		SpawnTrailEffects(EndTrace);
	}
}


void AVWeaponInstant::SpawnImpactEffects(const FHitResult& Impact)
{
	if (ImpactTemplate && Impact.bBlockingHit)
	{
		// TODO: Possible re-trace to get hit component that is lost during replication.

		/* This function prepares an actor to spawn, but requires another call to finish the actual spawn progress. This allows manipulation of properties before entering into the level */
		AVImpactEffect* EffectActor = GetWorld()->SpawnActorDeferred<AVImpactEffect>(ImpactTemplate, FTransform(Impact.ImpactPoint.Rotation(), Impact.ImpactPoint));
		if (EffectActor)
		{
			EffectActor->SurfaceHit = Impact;
			UGameplayStatics::FinishSpawningActor(EffectActor, FTransform(Impact.ImpactNormal.Rotation(), Impact.ImpactPoint));
		}
	}
}


void AVWeaponInstant::SpawnTrailEffects(const FVector& EndPoint)
{
	// Keep local count for effects
	BulletsShotCount++;

	const FVector Origin = GetMuzzleLocation();
	FVector ShootDir = EndPoint - Origin;

	// Only spawn if a minimum distance is satisfied.
	if (ShootDir.Size() < MinimumProjectileSpawnDistance)
	{
		return;
	}

	if (BulletsShotCount % TracerRoundInterval == 0)
	{
		if (TracerFX)
		{
			ShootDir.Normalize();
			UGameplayStatics::SpawnEmitterAtLocation(this, TracerFX, Origin, ShootDir.Rotation());
		}
	}
	else
	{
		// Only create trails FX by other players.
		AVCharacter* OwningPawn = GetPawnOwner();
		if (OwningPawn && OwningPawn->IsLocallyControlled())
		{
			return;
		}

		if (TrailFX)
		{
			UParticleSystemComponent* TrailPSC = UGameplayStatics::SpawnEmitterAtLocation(this, TrailFX, Origin);
			if (TrailPSC)
			{
				TrailPSC->SetVectorParameter(TrailTargetParam, EndPoint);
			}
		}
	}
}


void AVWeaponInstant::OnRep_HitLocation()
{
	// Played on all remote clients
	SimulateInstantHit(HitImpactNotify);
}


void AVWeaponInstant::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AVWeaponInstant, HitImpactNotify, COND_SkipOwner);
}
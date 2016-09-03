// copyright The Perfect Game Company 2016
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
	MinimumProjectileSpawnDistance = 800;
	TracerRoundInterval = 3;
	TraceParams = FCollisionQueryParams(FName(TEXT("HitscanTrace")), true, this);
	//TraceParams.TraceTag = FName("HitscanTrace");
}
void AVWeaponInstant::FireWeapon()
{
	FHitResult Hit(ForceInit);
	FVector Start = GetMuzzleLocation();
	FVector End = Start + CalcSpread() * WeaponRange;

	GetWorld()->LineTraceSingleByChannel(Hit, Start, End, COLLISION_WEAPON, TraceParams);

	//UE_LOG(LogTemp, Warning, TEXT("Fire!"));
	//CurrentAmmo--;
	ProcessInstantHit(Hit, Start, End);
	SimulateWeaponFire();
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
	return true;
}
void AVWeaponInstant::DealDamage(const FHitResult& Hit) {
	if (Hit.GetActor()) {
		float ActualHitDamage = HitDamage;
		 
		/*UVDamageType* DmgType = Cast<UVDamageType>(DamageType->GetDefaultObject(true));
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
void AVWeaponInstant::ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir)
{
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
	
	// Play FX locally
	SimulateInstantHit(Impact.ImpactPoint);
	
}
void AVWeaponInstant::SimulateInstantHit(const FVector& ImpactPoint)
{
	const FVector MuzzleOrigin = GetMuzzleLocation();

	/* Adjust direction based on desired cross hair impact point and muzzle location */
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




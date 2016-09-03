// copyright The Perfect Game Company 2016
#pragma once

#include "VWeapon.h"
#include "VWeaponInstant.generated.h"

/**
 * Base class for instant damage weapons assault rifles pistols machine guns etc.
 */
UCLASS(ABSTRACT, Blueprintable)
class VIM_API AVWeaponInstant : public AVWeapon
{
	GENERATED_BODY()


protected:

	AVWeaponInstant(const FObjectInitializer& ObjectInitializer);

	/************************************************************************/
	/* Damage Processing                                                    */
	/************************************************************************/

	virtual void FireWeapon() override;

	FVector CalcSpread();

	void DealDamage(const FHitResult& Impact);

	bool ShouldDealDamage(AActor* TestActor) const;

	void ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir);

	void ProcessInstantHitConfirmed(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir);

	/************************************************************************/
	/* Weapon Configuration                                                 */
	/************************************************************************/

	UPROPERTY(EditDefaultsOnly)
		float HitDamage;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<class UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly)
		float WeaponRange;

	UPROPERTY(EditDefaultsOnly)
		float Spread = 0.0f;



private:

	/************************************************************************/
	/* Visual Handlers                                                      */
	/************************************************************************/

	void SimulateInstantHit(const FVector& ImpactPoint);

	void SpawnImpactEffects(const FHitResult& Impact);

	void SpawnTrailEffects(const FVector& EndPoint);

	/* Particle FX played when a surface is hit. */
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<class AVImpactEffect> ImpactTemplate;

	UPROPERTY(EditDefaultsOnly)
		FName TrailTargetParam;

	UPROPERTY(EditDefaultsOnly)
		UParticleSystem* TrailFX;

	UPROPERTY(EditDefaultsOnly)
		UParticleSystem* TracerFX;

	/* Minimum firing distance before spawning tracers or trails. */
	UPROPERTY(EditDefaultsOnly)
		float MinimumProjectileSpawnDistance;

	UPROPERTY(EditDefaultsOnly)
		int32 TracerRoundInterval;

	/* Keeps track of number of shots fired */
	int32 BulletsShotCount;

	FCollisionQueryParams TraceParams;


	
};

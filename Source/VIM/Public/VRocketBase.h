// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "VRocketBase.generated.h"

UCLASS()
class VIM_API AVRocketBase : public AActor
{
	GENERATED_BODY()
	
public:
	AVRocketBase();
	void Inititalize(FVector Direction);

	void Trace(float DeltaTime);
	void Move(float DeltaTime);
	void DealDamage(FHitResult& Hit);
	void SpawnImpactEffect(FHitResult& Hit);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ProjectileBase, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* ProjectileMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ProjectileBase, meta = (AllowPrivateAccess = "true"))
		UParticleSystemComponent* ProjectileParticles;

private:
	UPROPERTY(EditDefaultsOnly)
		float InitialSpeed;
	UPROPERTY(EditDefaultsOnly)
		float ImpulseStrength;
	UPROPERTY(EditDefaultsOnly)
		USoundCue* ImpactSound;
	UPROPERTY(EditDefaultsOnly)
		UParticleSystem* ImpactEffect;
	UPROPERTY(EditDefaultsOnly)
		FRadialDamageParams RadDamageParams;

	FVector Velocity;
	FCollisionQueryParams TraceParams;
};



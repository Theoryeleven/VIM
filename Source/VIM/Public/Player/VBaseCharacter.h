// copyright The Perfect Game Company 2016

#pragma once

#include "GameFramework/Character.h"
#include "Types.h"
#include "VBaseCharacter.generated.h"

UCLASS(ABSTRACT)
class VIM_API AVBaseCharacter : public ACharacter
{
	GENERATED_BODY()

	/* Tracks noise data used by the pawn sensing component */
	UPawnNoiseEmitterComponent* NoiseEmitterComp;

public:
	// Sets default values for this character's properties
	AVBaseCharacter(const class FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
		USoundCue* SoundTakeHit;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
		USoundCue* SoundDeath;

	/************************************************************************/
	/* Health                                                               */
	/************************************************************************/

	UFUNCTION(BlueprintCallable, Category = "PlayerCondition")
		float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerCondition")
		float GetHealth() const;

	UFUNCTION(BlueprintPure, Category = "PlayerCondition")
		float GetHealthPercentage() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerCondition")
		bool IsAlive() const;

	/************************************************************************/
	/* Shields                                                              */
	/************************************************************************/

	UFUNCTION(BlueprintCallable, Category = "PlayerCondition")
		float GetShields() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerCondition")
		float GetMaxShields() const;

	UFUNCTION(BlueprintPure, Category = "PlayerCondition")
		float GetShieldPercentage() const;
	/* Limit to trigger shield failure warning */
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCondition")
		float CriticalShieldThreshold;

	UPROPERTY(EditDefaultsOnly, Category = "PlayerCondition", Replicated)
		float Shields;

	// Documentation Note: MaxHunger does not need to be replicated, only values that change and are displayed or used by clients should ever be replicated.
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCondition")
		float MaxShields;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
		float SprintingSpeedModifier;

	/* Character wants to run, checked during Tick to see if allowed */
	UPROPERTY(Transient, Replicated)
		bool bWantsToRun;


	
	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
		float TargetingSpeedModifier;

	/************************************************************************/
	/* Damage & Death                                                       */
	/************************************************************************/
	
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCondition", Replicated)
		float Health;

	/* Take damage & handle death */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	virtual bool CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser) const;

	virtual bool Die(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser);

	virtual void OnDeath(float KillingDamage, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser);

	virtual void FellOutOfWorld(const class UDamageType& DmgType) override;

	void SetRagdollPhysics();

	
	bool bIsDying;
};

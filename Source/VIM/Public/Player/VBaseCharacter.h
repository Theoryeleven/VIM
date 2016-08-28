// copyright The Perfect Game Company 2016

#pragma once

#include "GameFramework/Character.h"
#include "VBaseCharacter.generated.h"


UENUM()
enum class EInventorySlot : uint8
{
	/* For currently equipped items/weapons */
	Hands,

	/* For primary weapons on spine bone */
	Primary,

	/* Storage for small items like flashlight on pelvis */
	Secondary,
};


UENUM()
enum class EBotBehaviorType : uint8
{
	/* Does not move, remains in place until a player is spotted */
	Passive,

	/* Patrols a region until a player is spotted */
	Patrolling,
};


USTRUCT()
struct FTakeHitInfo
{
	GENERATED_BODY()

		UPROPERTY()
		float ActualDamage;

	UPROPERTY()
		UClass* DamageTypeClass;

	UPROPERTY()
		TWeakObjectPtr<class AVBaseCharacter> PawnInstigator;

	UPROPERTY()
		TWeakObjectPtr<class AActor> DamageCauser;

	UPROPERTY()
		uint8 DamageEventClassID;

	UPROPERTY()
		bool bKilled;

private:

	UPROPERTY()
		uint8 EnsureReplicationByte;

	UPROPERTY()
		FDamageEvent GeneralDamageEvent;

	UPROPERTY()
		FPointDamageEvent PointDamageEvent;

	UPROPERTY()
		FRadialDamageEvent RadialDamageEvent;

public:
	FTakeHitInfo()
		: ActualDamage(0),
		DamageTypeClass(nullptr),
		PawnInstigator(nullptr),
		DamageCauser(nullptr),
		DamageEventClassID(0),
		bKilled(false),
		EnsureReplicationByte(0)
	{}

	FDamageEvent& GetDamageEvent()
	{
		switch (DamageEventClassID)
		{
		case FPointDamageEvent::ClassID:
			if (PointDamageEvent.DamageTypeClass == nullptr)
			{
				PointDamageEvent.DamageTypeClass = DamageTypeClass ? DamageTypeClass : UDamageType::StaticClass();
			}
			return PointDamageEvent;

		case FRadialDamageEvent::ClassID:
			if (RadialDamageEvent.DamageTypeClass == nullptr)
			{
				RadialDamageEvent.DamageTypeClass = DamageTypeClass ? DamageTypeClass : UDamageType::StaticClass();
			}
			return RadialDamageEvent;

		default:
			if (GeneralDamageEvent.DamageTypeClass == nullptr)
			{
				GeneralDamageEvent.DamageTypeClass = DamageTypeClass ? DamageTypeClass : UDamageType::StaticClass();
			}
			return GeneralDamageEvent;
		}
	}


	void SetDamageEvent(const FDamageEvent& DamageEvent)
	{
		DamageEventClassID = DamageEvent.GetTypeID();
		switch (DamageEventClassID)
		{
		case FPointDamageEvent::ClassID:
			PointDamageEvent = *((FPointDamageEvent const*)(&DamageEvent));
			break;
		case FRadialDamageEvent::ClassID:
			RadialDamageEvent = *((FRadialDamageEvent const*)(&DamageEvent));
			break;
		default:
			GeneralDamageEvent = DamageEvent;
		}
	}


	void EnsureReplication()
	{
		EnsureReplicationByte++;
	}
};

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

	UFUNCTION(BlueprintCallable, Category = "PlayerCondition")
		bool IsAlive() const;

	

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

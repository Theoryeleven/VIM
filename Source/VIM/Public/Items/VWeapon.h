// copyright The Perfect Game Company 2016

#pragma once

#include "GameFramework/Actor.h"
#include "Types.h"
#include "VCharacter.h"
#include "VWeapon.generated.h"

UENUM()
enum class EWeaponState
{
	Idle,
	Firing,
	Equipping,
	Reloading
};

USTRUCT()
struct FWeaponData
{
	GENERATED_USTRUCT_BODY()

		/** inifite ammo for reloads */
		UPROPERTY(EditDefaultsOnly, Category = Ammo)
		bool bInfiniteAmmo;

	/** infinite ammo in clip, no reload required */
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
		bool bInfiniteClip;

	/** max ammo */
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
		int32 MaxAmmo;

	/** clip size */
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
		int32 AmmoPerClip;

	/** initial clips */
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
		int32 InitialClips;

	/** time between two consecutive shots */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
		float TimeBetweenShots;

	/** failsafe reload duration if weapon doesn't have any animation for it */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
		float NoAnimReloadDuration;

	/** defaults */
	FWeaponData()
	{
		bInfiniteAmmo = false;
		bInfiniteClip = false;
		MaxAmmo = 100;
		AmmoPerClip = 20;
		InitialClips = 4;
		TimeBetweenShots = 0.2f;
		NoAnimReloadDuration = 1.0f;
	}
};

/**
 * Base weapon class all now weapon types are inherited from this class
 * YOU MUST DECLARE A FIREWEAPON CLASS IN YOUR INHERITED CLASSES !!!!!! 
 */
UCLASS(ABSTRACT, Blueprintable)
class VIM_API AVWeapon : public AActor
{
	GENERATED_BODY()

public:

	virtual void PostInitializeComponents() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	float GetEquipStartedTime() const;

	float GetEquipDuration() const;

	/** last time when this weapon was switched to */
	float EquipStartedTime;

	/** how much time weapon needs to be equipped */
	float EquipDuration;

	bool bIsEquipped;

	bool bPendingEquip;

	FTimerHandle TimerHandle_HandleFiring;

	FTimerHandle EquipFinishedTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ShotsPerMinute;

protected:

	AVWeapon(const FObjectInitializer& ObjectInitializer);

	/* The character socket to store this item at. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	EInventorySlot StorageSlot;

	/** pawn owner */
	UPROPERTY()
	class AVCharacter* MyPawn;

	/** weapon mesh: 3rd person view */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh;

	UFUNCTION()
	void OnRep_MyPawn();

	/** detaches weapon mesh from pawn */
	void DetachMeshFromPawn();

	virtual void OnEquipFinished();

	bool IsEquipped() const;

	bool IsAttachedToPawn() const;

public:

	/** get weapon mesh (needs pawn owner to determine variant) */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	USkeletalMeshComponent* GetWeaponMesh() const;

	virtual void OnUnEquip();

	void OnEquip(bool bPlayAnimation);

	/* Set the weapon's owning pawn */
	void SetOwningPawn(AVCharacter* NewOwner);

	/* Get pawn owner */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	class AVCharacter* GetPawnOwner() const;

	virtual void OnEnterInventory(AVCharacter* NewOwner);

	virtual void OnLeaveInventory();

	FORCEINLINE EInventorySlot GetStorageSlot()
	{
		return StorageSlot;
	}

	/* The class to spawn in the level when dropped */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<class AVWeaponPickup> WeaponPickupClass;



	/************************************************************************/
	/* Fire & Damage Handling                                               */
	/************************************************************************/

public:

	

	void StartFire();

	void StopFire();

	EWeaponState GetCurrentState() const;

	/* You can assign default values to function parameters, these are then optional to specify/override when calling the function. */
	void AttachMeshToPawn(EInventorySlot Slot = EInventorySlot::Hands);

protected:

	bool CanFire() const;

	FVector GetAdjustedAim() const;

	FVector GetCameraDamageStartLocation(const FVector& AimDir) const;

	FHitResult WeaponTrace(const FVector& TraceFrom, const FVector& TraceTo) const;

	/* With PURE_VIRTUAL we skip implementing the function in SWeapon.cpp and can do this in SWeaponInstant.cpp / SFlashlight.cpp instead */
	virtual void FireWeapon() PURE_VIRTUAL(AVWeapon::FireWeapon(),);

private:

	void SetWeaponState(EWeaponState NewState);

	void DetermineWeaponState();

	virtual void HandleFiring();

	void OnBurstStarted();

	void OnBurstFinished();

	bool bWantsToFire;

	EWeaponState CurrentState;

	bool bRefiring;

	float LastFireTime;

	/* Time between shots for repeating fire */
	float TimeBetweenShots;

	/************************************************************************/
	/* Simulation & FX                                                      */
	/************************************************************************/

private:

	UFUNCTION()
	void OnRep_BurstCounter();

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* FireSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* EquipSound;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* MuzzleFX;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* EquipAnim;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* FireAnim;

	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzlePSC;


	UPROPERTY(EditDefaultsOnly)
	FName MuzzleAttachPoint;

	bool bPlayingFireAnim;

	UPROPERTY()
	int32 BurstCounter;

protected:

	virtual void SimulateWeaponFire();

	virtual void StopSimulatingWeaponFire();

	FVector GetMuzzleLocation() const;

	FVector GetMuzzleDirection() const;

	UAudioComponent* PlayWeaponSound(USoundCue* SoundToPlay);

	float PlayWeaponAnimation(UAnimMontage* Animation, float InPlayRate = 1.f, FName StartSectionName = NAME_None);

	void StopWeaponAnimation(UAnimMontage* Animation);

	/************************************************************************/
	/* Ammo & Reloading                                                     */
	/************************************************************************/

private:

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* OutOfAmmoSound;

	FTimerHandle TimerHandle_ReloadWeapon;

	FTimerHandle TimerHandle_StopReload;

protected:

	enum class EAmmoType
	{
		EBullet,
		ERocket,
		EMax,
	};

	/* Time to assign on reload when no animation is found */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float NoAnimReloadDuration;

	/* Time to assign on equip when no animation is found */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float NoEquipAnimDuration;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_Reload)
	bool bPendingReload;

	void UseAmmo();

	UPROPERTY(Transient, Replicated)
	int32 CurrentAmmo;

	UPROPERTY(Transient, Replicated)
	int32 CurrentAmmoInClip;

	/* Weapon ammo on spawn */
	UPROPERTY(EditDefaultsOnly)
	int32 StartAmmo;

	UPROPERTY(EditDefaultsOnly)
	int32 MaxAmmo;

	UPROPERTY(EditDefaultsOnly)
	int32 MaxAmmoPerClip;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* ReloadSound;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* ReloadAnim;

	virtual void ReloadWeapon();

	/* Is weapon and character currently capable of starting a reload */
	bool CanReload();

	UFUNCTION()
	void OnRep_Reload();

public:

	class AVCharacter* OwningPlayer;

	virtual void StartReload(bool bFromReplication = false);

	virtual void StopSimulateReload();

	/* Give ammo to weapon and return the amount that was not 'consumed' beyond the max count */
	int32 GiveAmmo(int32 AddAmount);

	/* Set a new total amount of ammo of weapon */
	void SetAmmoCount(int32 NewTotalAmount);

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	int32 GetCurrentAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	int32 GetCurrentAmmoInClip() const;

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	int32 GetMaxAmmoPerClip() const;

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	int32 GetMaxAmmo() const;
};

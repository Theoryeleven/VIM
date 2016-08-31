// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "VBaseCharacter.h"
#include "VCharacter.generated.h"

UCLASS()
class VIM_API AVCharacter : public AVBaseCharacter
{
	GENERATED_BODY()

	AVCharacter(const FObjectInitializer& ObjectInitializer);
	
	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	virtual void BeginPlay() override;

	/* Called every frame */
	virtual void Tick(float DeltaSeconds) override;

	/** True if the controlled character should navigate to the mouse cursor. */
	uint32 bMoveToMouseCursor : 1;

	/** Navigate player to the current mouse cursor location. */
	void MoveToMouseCursor();

	/** Navigate player to the current touch location. */
	void MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location);

	/** Navigate player to the given world location. */
	void SetNewMoveDestination(const FVector DestLocation);

	/** Input handlers for SetDestination action. */
	void OnSetDestinationPressed();
	void OnSetDestinationReleased();

	/* Called to bind functionality to input */
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void PawnClientRestart() override;

	/* Stop playing all montages */
	void StopAllAnimMontages();

	float LastNoiseLoudness;

	float LastMakeNoiseTime;

private:

	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** A decal that projects to the cursor location. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UDecalComponent* CursorToWorld;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class UVCarryObjectComponent* CarriedObjectComp;

public:

	UFUNCTION(BlueprintCallable, Category = "AI")
	float GetLastNoiseLoudness();

	UFUNCTION(BlueprintCallable, Category = "AI")
	float GetLastMakeNoiseTime();

	FORCEINLINE UCameraComponent* GetCameraComponent()
	{
		return TopDownCameraComponent;
	}

	/* MakeNoise hook to trigger AI noise emitting (Loudness between 0.0-1.0)  */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void MakePawnNoise(float Loudness);

	/************************************************************************/
	/* Movement                                                             */
	/************************************************************************/

	

	virtual void SetSprinting(bool NewSprinting);

	/* Is character currently performing a jump action. Resets on landed.  */
	UPROPERTY(Transient, Replicated)
	bool bIsJumping;



	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

	/************************************************************************/
	/* Object Interaction                                                   */
	/************************************************************************/

	/* Input mapped function for carry object component */
	void OnToggleCarryActor();

	/* Use the usable actor currently in focus, if any */
	virtual void Use();

	

	class AVUsableActor* GetUsableInView();

	/*Max distance to use/focus on actors. */
	UPROPERTY(EditDefaultsOnly, Category = "ObjectInteraction")
	float MaxUseDistance;

	/* True only in first frame when focused on a new usable actor. */
	bool bHasNewFocus;

	class AVUsableActor* FocusedUsableActor;

	/************************************************************************/
	/* Targeting                                                            */
	/************************************************************************/


	/************************************************************************/
	/* Energy                                                               */
	/************************************************************************/

	UFUNCTION(BlueprintCallable, Category = "PlayerCondition")
	float GetEnergy() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerCondition")
	float GetMinEnergy() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerCondition")
	float GetMaxEnergy() const;

	UFUNCTION(BlueprintPure, Category = "PlayerCondition")
	float GetEnergyPercentage() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerCondition")
	void RestoreCondition(float HealthRestored, float EnergyRestored);

	/* Decrements Energy, used by timer. */
	void DecrimentEnergy();

	UPROPERTY(EditDefaultsOnly, Category = "PlayerCondition")
	float DecrimentEnergyInterval;

	UPROPERTY(EditDefaultsOnly, Category = "PlayerCondition")
	float DecrimentEnergyAmount;

	/* Limit when player suffers Hit points from extreme hunger */
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCondition")
	float CriticalEnergyThreshold;

	UPROPERTY(EditDefaultsOnly, Category = "PlayerCondition", Replicated)
	float Energy;

	// Documentation Note: MaxHunger does not need to be replicated, only values that change and are displayed or used by clients should ever be replicated.
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCondition")
	float MinEnergy;

	UPROPERTY(EditDefaultsOnly, Category = "PlayerCondition")
	float MaxEnergy;
	
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCondition")
	float EnergyDamagePerInterval;

	/* Damage type applied when player suffers critical Energy shortage */
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCondition")
	TSubclassOf<UDamageType> EnergyDamageType;

	

	/************************************************************************/
	/* Damage & Death                                                       */
	/************************************************************************/

	virtual void OnDeath(float KillingDamage, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser) override;

	virtual void Suicide();

	virtual void KilledBy(class APawn* EventInstigator);

	/************************************************************************/
	/* Weapons & Inventory                                                  */
	/************************************************************************/

private:

	/* Attachpoint for active weapon/item in hands */
	UPROPERTY(EditDefaultsOnly, Category = "Sockets")
	FName WeaponAttachPoint;

	/* Attachpoint for items carried on the belt/pelvis. */
	UPROPERTY(EditDefaultsOnly, Category = "Sockets")
	FName PelvisAttachPoint;

	/* Attachpoint for primary weapons */
	UPROPERTY(EditDefaultsOnly, Category = "Sockets")
	FName SpineAttachPoint;

	bool bWantsToFire;

	/* Distance away from character when dropping inventory items. */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	float DropWeaponMaxDistance;

	void OnReload();

	/* Mapped to input */
	void OnStartFire();

	/* Mapped to input */
	void OnStopFire();

	/* Mapped to input */
	void OnNextWeapon();

	/* Mapped to input */
	void OnPrevWeapon();

	/* Mapped to input */
	void OnEquipPrimaryWeapon();

	/* Mapped to input */
	void OnEquipSecondaryWeapon();

	void StartWeaponFire();

	void StopWeaponFire();

	void DestroyInventory();

	/* Mapped to input. Drops current weapon */
	void DropWeapon();



public:

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	AVWeapon* GetCurrentWeapon() const;

	/* Check if the specified slot is available, limited to one item per type (primary, secondary) */
	bool WeaponSlotAvailable(EInventorySlot CheckSlot);

	/* Check if pawn is allowed to fire weapon */
	bool CanFire() const;

	bool CanReload() const;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool IsFiring() const;

	/* Return socket name for attachments (to match the socket in the character skeleton) */
	FName GetInventoryAttachPoint(EInventorySlot Slot) const;

	/* All weapons/items the player currently holds */
	UPROPERTY(Transient, Replicated)
	TArray<AVWeapon*> Inventory;

	void SetCurrentWeapon(class AVWeapon* newWeapon, class AVWeapon* LastWeapon = nullptr);

	void EquipWeapon(AVWeapon* Weapon);



	/* OnRep functions can use a parameter to hold the previous value of the variable. Very useful when you need to handle UnEquip etc. */
	UFUNCTION()
	void OnRep_CurrentWeapon(AVWeapon* LastWeapon);

	void AddWeapon(class AVWeapon* Weapon);

	void RemoveWeapon(class AVWeapon* Weapon, bool bDestroy);

	UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentWeapon)
	class AVWeapon* CurrentWeapon;

	class AVWeapon* PreviousWeapon;

	/* Update the weapon mesh to the newly equipped weapon, this is triggered during an anim montage.
		NOTE: Requires an AnimNotify created in the Equip animation to tell us when to swap the meshes. */
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SwapToNewWeaponMesh();
};

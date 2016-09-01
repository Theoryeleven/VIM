// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "VIM.h"
#include "VCharacter.h"
#include "VUsableActor.h"
#include "VWeapon.h"
#include "VWeaponPickup.h"
#include "VCharacterMovementComponent.h"
#include "VCarryObjectComponent.h"
#include "VPlayerController.h"
#include "VBaseCharacter.h"
#include "AI/Navigation/NavigationSystem.h"


// Sets default values
AVCharacter::AVCharacter(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	/* Ignore this channel or it will absorb the trace impacts instead of the skeletal mesh */
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	
	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bAbsoluteRotation = true; // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->RelativeRotation = FRotator(-60.f, 0.f, 0.f);
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

										  // Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

															 // Create a decal in the world to show the cursor's location
	CursorToWorld = CreateDefaultSubobject<UDecalComponent>("CursorToWorld");
	CursorToWorld->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UMaterial> DecalMaterialAsset(TEXT("Material'/Game/VIM/Blueprints/M_Cursor_Decal.M_Cursor_Decal'"));
	if (DecalMaterialAsset.Succeeded())
	{
		CursorToWorld->SetDecalMaterial(DecalMaterialAsset.Object);
	}
	CursorToWorld->DecalSize = FVector(16.0f, 32.0f, 32.0f);
	CursorToWorld->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());
	
	
	CarriedObjectComp = ObjectInitializer.CreateDefaultSubobject<UVCarryObjectComponent>(this, TEXT("CarriedObjectComp"));
	CarriedObjectComp->SetupAttachment(GetRootComponent());

	MaxUseDistance = 500;
	DropWeaponMaxDistance = 100;
	bHasNewFocus = true;
	TargetingSpeedModifier = 0.5f;
	SprintingSpeedModifier = 2.5f;

	Health = 100;

	IncrimentEnergyAmount = 5.0f;
	IncrimentEnergyInterval = 5.0f;
	CriticalEnergyThreshold = 10;
	EnergyDamagePerInterval = 1.0f;
	MinEnergy = 0;
	MaxEnergy = 100;
	Energy = 100;
	Shields = 0;
	MaxShields = 100;

	/* Names as specified in the character skeleton */
	WeaponAttachPoint = TEXT("WeaponSocket");
	PelvisAttachPoint = TEXT("PelvisSocket");
	SpineAttachPoint = TEXT("SpineSocket");
}


void AVCharacter::BeginPlay()
{
	Super::BeginPlay();

	
		FTimerHandle Handle;
		GetWorldTimerManager().SetTimer(Handle, this, &AVCharacter::IncrimentEnergy, IncrimentEnergyInterval, true);
	
}


void AVCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (CursorToWorld != nullptr)
	{
		
		if (AVPlayerController *PC = Cast<AVPlayerController>(GetController()))
		{
			FHitResult TraceHitResult;
			PC->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);
			FVector CursorFV = TraceHitResult.ImpactNormal;
			FRotator CursorR = CursorFV.Rotation();
			CursorToWorld->SetWorldLocation(TraceHitResult.Location);
			CursorToWorld->SetWorldRotation(CursorR);
		}
	}
	if (bMoveToMouseCursor)
	{
		MoveToMouseCursor();
	}
}

void AVCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	DestroyInventory();
}


// Called to bind functionality to input
void AVCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);
	
	// Movement
	InputComponent->BindAction("SetDestination", IE_Pressed, this, &AVCharacter::OnSetDestinationPressed);
	InputComponent->BindAction("SetDestination", IE_Released, this, &AVCharacter::OnSetDestinationReleased);

	// support touch devices 
	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AVCharacter::MoveToTouchLocation);
	InputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AVCharacter::MoveToTouchLocation);

	// Interaction
	InputComponent->BindAction("Use", IE_Pressed, this, &AVCharacter::Use);
	InputComponent->BindAction("DropWeapon", IE_Pressed, this, &AVCharacter::DropWeapon);

	// Weapons
	InputComponent->BindAction("Fire", IE_Pressed, this, &AVCharacter::OnStartFire);
	InputComponent->BindAction("Fire", IE_Released, this, &AVCharacter::OnStopFire);

	InputComponent->BindAction("Reload", IE_Pressed, this, &AVCharacter::OnReload);

	InputComponent->BindAction("NextWeapon", IE_Pressed, this, &AVCharacter::OnNextWeapon);
	InputComponent->BindAction("PrevWeapon", IE_Pressed, this, &AVCharacter::OnPrevWeapon);

	InputComponent->BindAction("EquipPrimaryWeapon", IE_Pressed, this, &AVCharacter::OnEquipPrimaryWeapon);
	InputComponent->BindAction("EquipSecondaryWeapon", IE_Pressed, this, &AVCharacter::OnEquipSecondaryWeapon);

	/* Input binding for the carry object component */
	InputComponent->BindAction("PickupObject", IE_Pressed, this, &AVCharacter::OnToggleCarryActor);
}
/*
Performs ray-trace to find closest looked-at UsableActor.
*/
AVUsableActor* AVCharacter::GetUsableInView()
{
	FVector CamLoc;
	FRotator CamRot;

	if (Controller == nullptr)
		return nullptr;

	Controller->GetPlayerViewPoint(CamLoc, CamRot);
	const FVector TraceStart = CamLoc;
	const FVector Direction = CamRot.Vector();
	const FVector TraceEnd = TraceStart + (Direction * MaxUseDistance);

	FCollisionQueryParams TraceParams(TEXT("TraceUsableActor"), true, this);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = false;

	/* Not tracing complex uses the rough collision instead making tiny objects easier to select. */
	TraceParams.bTraceComplex = false;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams);

	//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 1.0f);

	return Cast<AVUsableActor>(Hit.GetActor());
}
void AVCharacter::Use()
{
	
		AVUsableActor* Usable = GetUsableInView();
		if (Usable)
		{
			Usable->OnUsed(this);
		}
	
}
void AVCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	/* Check if we are no longer falling/jumping */
	
}
void AVCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to every client, no special condition required
	DOREPLIFETIME(AVCharacter, Energy);
	DOREPLIFETIME(AVCharacter, CurrentWeapon);
	DOREPLIFETIME(AVCharacter, Inventory);
	/* If we did not display the current inventory on the player mesh we could optimize replication by using this replication condition. */
	/* DOREPLIFETIME_CONDITION(ASCharacter, Inventory, COND_OwnerOnly);*/
}
float AVCharacter::GetXPS() const
{
	return EXPS;
}
void AVCharacter::AddEXPS(int32 kill)
{
	EXPS = EXPS + kill;
}
float AVCharacter::GetEnergy() const
{
	return Energy;
}
float AVCharacter::GetMinEnergy() const
{
	return MinEnergy;
}
float AVCharacter::GetMaxEnergy() const
{
	return MaxEnergy;
}
float AVCharacter::GetEnergyPercentage() const
{
	return Energy / MaxEnergy;
}
void AVCharacter::RestoreCondition(float HealthRestored, float EnergyRestored)
{
	// Reduce Hunger, ensure we do not go outside of our bounds
	Energy = FMath::Clamp(Energy - EnergyRestored, 0.0f, GetMinEnergy());

	// Restore Hitpoints
	Health = FMath::Clamp(Health + HealthRestored, 0.0f, GetMaxHealth());

	AVPlayerController* PC = Cast<AVPlayerController>(Controller);
	if (PC)
	{
		PC->HUDMessage(EHUDMessage::Character_EnergyRestored);
	}
}
void AVCharacter::IncrimentEnergy()
{
	Energy = FMath::Clamp(Energy + IncrimentEnergyAmount, 0.0f, GetMaxEnergy());

	if (Energy < CriticalEnergyThreshold)
	{
		FDamageEvent DmgEvent;
		DmgEvent.DamageTypeClass = EnergyDamageType;

		// Apply damage to self.
		TakeDamage(EnergyDamagePerInterval, DmgEvent, GetController(), this);
	}
}
void AVCharacter::OnDeath(float KillingDamage, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser)
{
	if (bIsDying)
	{
		return;
	}

	DestroyInventory();
	StopAllAnimMontages();

	Super::OnDeath(KillingDamage, DamageEvent, PawnInstigator, DamageCauser);
}
bool AVCharacter::CanFire() const
{
	/* Add your own checks here, for example non-shooting areas or checking if player is in an NPC dialogue etc. */
	return IsAlive();
}
bool AVCharacter::CanReload() const
{
	return IsAlive();
}
bool AVCharacter::IsFiring() const
{
	return CurrentWeapon && CurrentWeapon->GetCurrentState() == EWeaponState::Firing;
}
FName AVCharacter::GetInventoryAttachPoint(EInventorySlot Slot) const
{
	/* Return the socket name for the specified storage slot */
	switch (Slot)
	{
	case EInventorySlot::Hands:
		return WeaponAttachPoint;
	case EInventorySlot::Primary:
		return SpineAttachPoint;
	case EInventorySlot::Secondary:
		return PelvisAttachPoint;
	default:
		// Not implemented.
		return "";
	}
}
void AVCharacter::DestroyInventory()
{
	for (int32 i = Inventory.Num() - 1; i >= 0; i--)
	{
		AVWeapon* Weapon = Inventory[i];
		if (Weapon)
		{
			RemoveWeapon(Weapon, true);
		}
	}
}
void AVCharacter::SetCurrentWeapon(class AVWeapon* NewWeapon, class AVWeapon* LastWeapon)
{
	/* Maintain a reference for visual weapon swapping */
	PreviousWeapon = LastWeapon;

	AVWeapon* LocalLastWeapon = nullptr;
	if (LastWeapon)
	{
		LocalLastWeapon = LastWeapon;
	}
	else if (NewWeapon != CurrentWeapon)
	{
		LocalLastWeapon = CurrentWeapon;
	}

	// UnEquip the current
	bool bHasPreviousWeapon = false;
	if (LocalLastWeapon)
	{
		LocalLastWeapon->OnUnEquip();
		bHasPreviousWeapon = true;
	}

	CurrentWeapon = NewWeapon;

	if (NewWeapon)
	{
		NewWeapon->SetOwningPawn(this);
		/* Only play equip animation when we already hold an item in hands */
		NewWeapon->OnEquip(bHasPreviousWeapon);
	}

	/* NOTE: If you don't have an equip animation w/ animnotify to swap the meshes halfway through, then uncomment this to immediately swap instead */
	//SwapToNewWeaponMesh();
}
void AVCharacter::OnRep_CurrentWeapon(AVWeapon* LastWeapon)
{
	SetCurrentWeapon(CurrentWeapon, LastWeapon);
}
AVWeapon* AVCharacter::GetCurrentWeapon() const
{
	return CurrentWeapon;
}
void AVCharacter::EquipWeapon(AVWeapon* Weapon)
{
	if (Weapon)
	{
		/* Ignore if trying to equip already equipped weapon */
		if (Weapon == CurrentWeapon)
			return;

		
		SetCurrentWeapon(Weapon, CurrentWeapon);
		
	}
}
void AVCharacter::AddWeapon(class AVWeapon* Weapon)
{
		Weapon->OnEnterInventory(this);
		Inventory.AddUnique(Weapon);

		// Equip first weapon in inventory
		if (Inventory.Num() > 0 && CurrentWeapon == nullptr)
		{
			EquipWeapon(Inventory[0]);
		}
}
void AVCharacter::RemoveWeapon(class AVWeapon* Weapon, bool bDestroy)
{
		bool bIsCurrent = CurrentWeapon == Weapon;

		if (Inventory.Contains(Weapon))
		{
			Weapon->OnLeaveInventory();
		}
		Inventory.RemoveSingle(Weapon);

		/* Replace weapon if we removed our current weapon */
		if (bIsCurrent && Inventory.Num() > 0)
		{
			SetCurrentWeapon(Inventory[0]);
		}			

		/* Clear reference to weapon if we have no items left in inventory */
		if (Inventory.Num() == 0)
		{
			SetCurrentWeapon(nullptr);
		}

		if (bDestroy)
		{
			Weapon->Destroy();
		}
}
void AVCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	/* Equip the weapon on the client side. */
	SetCurrentWeapon(CurrentWeapon);
}
void AVCharacter::OnReload()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartReload();
	}
}
void AVCharacter::OnStartFire()
{
	StartWeaponFire();
}
void AVCharacter::OnStopFire()
{
	StopWeaponFire();
}
void AVCharacter::StartWeaponFire()
{
	if (!bWantsToFire)
	{
		bWantsToFire = true;
		if (CurrentWeapon)
		{
			CurrentWeapon->StartFire();
		}
	}
}
void AVCharacter::StopWeaponFire()
{
	if (bWantsToFire)
	{
		bWantsToFire = false;
		if (CurrentWeapon)
		{
			CurrentWeapon->StopFire();
		}
	}
}
void AVCharacter::OnNextWeapon()
{
	if (CarriedObjectComp->GetIsCarryingActor())
	{
		CarriedObjectComp->Rotate(0.0f, 1.0f);
		return;
	}

	if (Inventory.Num() >= 2) // TODO: Check for weaponstate.
	{
		const int32 CurrentWeaponIndex = Inventory.IndexOfByKey(CurrentWeapon);
		AVWeapon* NextWeapon = Inventory[(CurrentWeaponIndex + 1) % Inventory.Num()];
		EquipWeapon(NextWeapon);
	}
}
void AVCharacter::OnPrevWeapon()
{
	if (CarriedObjectComp->GetIsCarryingActor())
	{
		CarriedObjectComp->Rotate(0.0f, -1.0f);
		return;
	}

	if (Inventory.Num() >= 2) // TODO: Check for weaponstate.
	{
		const int32 CurrentWeaponIndex = Inventory.IndexOfByKey(CurrentWeapon);
		AVWeapon* PrevWeapon = Inventory[(CurrentWeaponIndex - 1 + Inventory.Num()) % Inventory.Num()];
		EquipWeapon(PrevWeapon);
	}
}
void AVCharacter::DropWeapon()
{
	if (CurrentWeapon)
	{
		FVector CamLoc;
		FRotator CamRot;

		if (Controller == nullptr)
		{
			return;
		}		
		
		/* Find a location to drop the item, slightly in front of the player.
			Perform ray trace to check for blocking objects or walls and to make sure we don't drop any item through the level mesh */
		Controller->GetPlayerViewPoint(CamLoc, CamRot);
		FVector SpawnLocation;
		FRotator SpawnRotation = CamRot;

		const FVector Direction = CamRot.Vector();
		const FVector TraceStart = GetActorLocation();
		const FVector TraceEnd = GetActorLocation() + (Direction * DropWeaponMaxDistance);

		/* Setup the trace params, we are only interested in finding a valid drop position */
		FCollisionQueryParams TraceParams;
		TraceParams.bTraceComplex = false;
		TraceParams.bReturnPhysicalMaterial = false;
		TraceParams.AddIgnoredActor(this);

		FHitResult Hit;
		GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldDynamic, TraceParams);

		/* Find farthest valid spawn location */
		if (Hit.bBlockingHit)
		{
			/* Slightly move away from impacted object */
			SpawnLocation = Hit.ImpactPoint + (Hit.ImpactNormal * 20);
		}
		else
		{
			SpawnLocation = TraceEnd;
		}

		/* Spawn the "dropped" weapon */
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AVWeaponPickup* NewWeaponPickup = GetWorld()->SpawnActor<AVWeaponPickup>(CurrentWeapon->WeaponPickupClass, SpawnLocation, FRotator::ZeroRotator, SpawnInfo);

		if (NewWeaponPickup)
		{
			/* Apply torque to make it spin when dropped. */
			UStaticMeshComponent* MeshComp = NewWeaponPickup->GetMeshComponent();
			if (MeshComp)
			{
				MeshComp->SetSimulatePhysics(true);
				MeshComp->AddTorque(FVector(1, 1, 1) * 4000000);
			}
		}

		RemoveWeapon(CurrentWeapon, true);
	}
}
void AVCharacter::OnEquipPrimaryWeapon()
{
	if (CarriedObjectComp->GetIsCarryingActor())
	{
		CarriedObjectComp->Rotate(1.0f, 0.0f);
		return;
	}

	if (Inventory.Num() >= 1)
	{
		/* Find first weapon that uses primary slot. */
		for (int32 i = 0; i < Inventory.Num(); i++)
		{
			AVWeapon* Weapon = Inventory[i];
			if (Weapon->GetStorageSlot() == EInventorySlot::Primary)
			{
				EquipWeapon(Weapon);
			}
		}
	}
}
void AVCharacter::OnEquipSecondaryWeapon()
{
	if (CarriedObjectComp->GetIsCarryingActor())
	{
		CarriedObjectComp->Rotate(-1.0f, 0.0f);
		return;
	}

	if (Inventory.Num() >= 2)
	{
		/* Find first weapon that uses secondary slot. */
		for (int32 i = 0; i < Inventory.Num(); i++)
		{
			AVWeapon* Weapon = Inventory[i];
			if (Weapon->GetStorageSlot() == EInventorySlot::Secondary)
			{
				EquipWeapon(Weapon);
			}
		}
	}
}
bool AVCharacter::WeaponSlotAvailable(EInventorySlot CheckSlot)
{
	/* Iterate all weapons to see if requested slot is occupied */
	for (int32 i = 0; i < Inventory.Num(); i++)
	{
		AVWeapon* Weapon = Inventory[i];
		if (Weapon)
		{
			if (Weapon->GetStorageSlot() == CheckSlot)
				return false;
		}
	}

	return true;

	/* Special find function as alternative to looping the array and performing if statements 
		the [=] prefix means "capture by value", other options include [] "capture nothing" and [&] "capture by reference" */
	//return nullptr == Inventory.FindByPredicate([=](ASWeapon* W){ return W->GetStorageSlot() == CheckSlot; });
}
void AVCharacter::StopAllAnimMontages()
{
	USkeletalMeshComponent* UseMesh = GetMesh();
	if (UseMesh && UseMesh->AnimScriptInstance)
	{
		UseMesh->AnimScriptInstance->Montage_Stop(0.0f);
	}
}
void AVCharacter::MakePawnNoise(float Loudness)
{
	MakeNoise(Loudness, this, GetActorLocation());
	LastNoiseLoudness = Loudness;
	LastMakeNoiseTime = GetWorld()->GetTimeSeconds();
}
float AVCharacter::GetLastNoiseLoudness()
{
	return LastNoiseLoudness;
}
float AVCharacter::GetLastMakeNoiseTime()
{
	return LastMakeNoiseTime;
}
void AVCharacter::Suicide()
{
	KilledBy(this);
}
void AVCharacter::KilledBy(class APawn* EventInstigator)
{
	if (!bIsDying)
	{
		AController* Killer = nullptr;
		if (EventInstigator != nullptr)
		{
			Killer = EventInstigator->Controller;
			LastHitBy = nullptr;
		}

		Die(Health, FDamageEvent(UDamageType::StaticClass()), Killer, nullptr);
	}
}
void AVCharacter::OnToggleCarryActor()
{
	CarriedObjectComp->Pickup();
}
void AVCharacter::SwapToNewWeaponMesh()
{
	if (PreviousWeapon)
	{
		PreviousWeapon->AttachMeshToPawn(PreviousWeapon->GetStorageSlot());
	}

	if (CurrentWeapon)
	{
		CurrentWeapon->AttachMeshToPawn(EInventorySlot::Hands);
	}
}
void AVCharacter::SetSprinting(bool NewSprinting)
{
	// impliment changes to character actions while they move at an increased rate
}
void AVCharacter::MoveToMouseCursor()
{

	// Trace to see what is under the mouse cursor
	FHitResult Hit;
	AVPlayerController *PC = Cast<AVPlayerController>(GetController());
	if (PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	{
		if (Hit.bBlockingHit)
		{

			// We hit something, move there
			SetNewMoveDestination(Hit.ImpactPoint);
		}
	}
}

void AVCharacter::MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	FVector2D ScreenSpaceLocation(Location);

	// Trace to see what is under the touch location
	FHitResult HitResult;
	AVPlayerController *PC = Cast<AVPlayerController>(GetController());
	if (PC->GetHitResultAtScreenPosition(ScreenSpaceLocation, ECC_Visibility, true, HitResult))
	{
		if (HitResult.bBlockingHit)
		{
			// We hit something, move there
			SetNewMoveDestination(HitResult.ImpactPoint);
		}
	}
}

void AVCharacter::SetNewMoveDestination(const FVector DestLocation)
{
	APawn* const Pawn = this;
	if (Pawn)
	{
		UNavigationSystem* const NavSys = GetWorld()->GetNavigationSystem();
		float const Distance = FVector::Dist(DestLocation, Pawn->GetActorLocation());
		AVPlayerController *PC = Cast<AVPlayerController>(GetController());
		// We need to issue move command only if far enough in order for walk animation to play correctly
		if (NavSys && (Distance > 120.0f))
		{
			NavSys->SimpleMoveToLocation(PC, DestLocation);

		}
	}
}

void AVCharacter::OnSetDestinationPressed()
{

		// set flag to keep updating destination until released
	bMoveToMouseCursor = true;
}

void AVCharacter::OnSetDestinationReleased()
{
	// clear flag to indicate we should stop updating the destination
	bMoveToMouseCursor = false;
}


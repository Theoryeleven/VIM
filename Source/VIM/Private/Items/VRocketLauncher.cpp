// Fill out your copyright notice in the Description page of Project Settings.

#include "VIM.h"
#include "VRocketLauncher.h"
#include "VProjectileBase.h"
#include "VCharacter.h"

void AVRocketLauncher::FireWeapon() {
	FVector Location = GetMuzzleLocation();
	FRotator Rotation = GetMuzzleLocation().Rotation();

	AVProjectileBase* Projectile = GetWorld()->SpawnActor<AVProjectileBase>(ProjectileClass, Location, Rotation);

	CurrentAmmo--;
	SimulateWeaponFire();
	MyPawn->GetCharacterMovement()->AddImpulse(-Rotation.Vector()*ImpulseStrength, true);

	if (Projectile) {
		Projectile->SetOwner(this);
		Projectile->Instigator = MyPawn;
		Projectile->Inititalize(Rotation.Vector());
	}

}


// Fill out your copyright notice in the Description page of Project Settings.

#include "VIM.h"
#include "TestRocketlauncher.h"
#include "VRocketBase.h"
#include "VCharacter.h"


void ATestRocketlauncher::FireWeapon() {
	FVector Location = GetMuzzleLocation();
	FRotator Rotation = GetMuzzleDirection().Rotation();

	AVRocketBase* Projectile = GetWorld()->SpawnActor<AVRocketBase>(ProjectileClass, Location, Rotation);

	CurrentAmmo--;
	SimulateWeaponFire();
	OwningPlayer->GetCharacterMovement()->AddImpulse(-Rotation.Vector()*ImpulseStrength, true);

	if (Projectile) {
		Projectile->SetOwner(this);
		Projectile->Instigator = OwningPlayer;
		Projectile->Inititalize(Rotation.Vector());
	}

}



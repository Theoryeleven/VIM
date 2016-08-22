// copyright The Perfect Game Company 2016

#include "VIM.h"
#include "AI/Navigation/NavigationSystem.h"
#include "VIMAIController.h"

AVIMAIController::AVIMAIController()
{

}

void AVIMAIController::BeginPlay()
{
	Super::BeginPlay();
	switch (SetAIType)
	{
	case EAIType::Bomber:
	{

	}
		break;
	case EAIType::Melee: 
	{

	}
		break;
	case EAIType::Sneaky:
	{

	}
		break;
	case EAIType::Mage:
	{

	}
		break;
	case EAIType::Ranged:
	{

	}
		break;
	default:
		break;
	}
}


void AVIMAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// keep updating the destination every tick while desired
	
}

void AVIMAIController::MoveToDestination()
{
	auto PlayerCharacter = (GetWorld()->GetFirstPlayerController()->GetPawn());
	auto ControlledCharacter = GetPawn();
	if (!ensure(PlayerCharacter && ControlledCharacter)) { return; }
	SetNewMoveDestination(PlayerCharacter->GetActorLocation());

}

void AVIMAIController::SetNewMoveDestination(const FVector DestLocation)
{
	APawn* const Pawn = GetPawn();
	if (Pawn)
	{
		UNavigationSystem* const NavSys = GetWorld()->GetNavigationSystem();
		float const Distance = FVector::Dist(DestLocation, Pawn->GetActorLocation());

		// We need to issue move command only if far enough in order for walk animation to play correctly
		if (NavSys && (Distance > 120.0f))
		{
			NavSys->SimpleMoveToLocation(this, DestLocation);
		}
	}
}


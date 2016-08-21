#include "VIM.h"
#include "AI/Navigation/NavigationSystem.h"
#include "VIMBasicAIController.h"


AVIMBasicAIController::AVIMBasicAIController()
{

}

void AVIMBasicAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// keep updating the destination every tick while desired
	MoveToDestination();
}

void AVIMBasicAIController::MoveToDestination()
{
	auto PlayerCharacter = (GetWorld()->GetFirstPlayerController()->GetPawn());
	auto ControlledCharacter = GetPawn();
	if (!ensure(PlayerCharacter && ControlledCharacter)) { return; }
	SetNewMoveDestination(PlayerCharacter->GetActorLocation());
	
}

void AVIMBasicAIController::SetNewMoveDestination(const FVector DestLocation)
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

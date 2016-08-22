// copyright The Perfect Game Company 2016

#pragma once

#include "AIController.h"
#include "VIMBasicAIController.generated.h"

/**
* Basic AI functionality 
*/

UCLASS()
class VIM_API AVIMBasicAIController : public AAIController
{
	GENERATED_BODY()

public:
	AVIMBasicAIController();

protected:
	// Begin PlayerController interface
	virtual void Tick(float DeltaTime) override;

private:
	uint32 bMoveToPlayer : 1;

	/** Navigate AI to the current Player location. */
	void MoveToDestination();

	/** Navigate AI to the given world location. */
	void SetNewMoveDestination(const FVector DestLocation);
};

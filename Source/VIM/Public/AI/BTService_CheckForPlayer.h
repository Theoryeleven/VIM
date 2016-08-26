// copyright The Perfect Game Company 2016

#pragma once

#include "BehaviorTree/BTService.h"
#include "BTService_CheckForPlayer.generated.h"

/**
*
*/

UCLASS()
class VIM_API UBTService_CheckForPlayer : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_CheckForPlayer();

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:


private:

};
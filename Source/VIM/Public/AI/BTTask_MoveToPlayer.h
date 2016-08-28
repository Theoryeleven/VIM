// copyright The Perfect Game Company 2016

#pragma once

#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_MoveToPlayer.generated.h"

/**
 * 
 */
UCLASS()
class VIM_API UBTTask_MoveToPlayer : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);
	
	
};

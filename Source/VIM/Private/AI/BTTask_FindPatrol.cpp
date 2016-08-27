// copyright The Perfect Game Company 2016

#include "VIM.h"
#include "BTTask_FindPatrol.h"
#include "VWayPoint.h"
#include "VAIController.h"

/* AI Module includes */
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
/* This contains includes all key types like UBlackboardKeyType_Vector used below. */
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"

EBTNodeResult::Type UBTTask_FindPatrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AVAIController* MyController = Cast<AVAIController>(OwnerComp.GetAIOwner());
	if (MyController == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	AVWayPoint* MyWaypoint = MyController->GetWaypoint();
	if (MyWaypoint)
	{
		/* Find a position that is close to the waypoint. We add a small random to this position to give build predictable patrol patterns  */
		const float SearchRadius = 300.0f;
		const FVector SearchOrigin = MyWaypoint->GetActorLocation();
		const FVector Loc = UNavigationSystem::GetRandomPointInNavigableRadius(MyController, SearchOrigin, SearchRadius);
		if (Loc != FVector::ZeroVector)
		{
			/* The selected key should be "PatrolLocation" in the BehaviorTree setup */
			OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Vector>(BlackboardKey.GetSelectedKeyID(), Loc);
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}
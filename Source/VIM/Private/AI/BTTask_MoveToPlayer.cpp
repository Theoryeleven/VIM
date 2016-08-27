// copyright The Perfect Game Company 2016

#include "VIM.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"
#include "VIMMobb.h"
#include "VIMCharacter.h"
#include "EnemyAI.h"
#include "BTTask_MoveToPlayer.h"

EBTNodeResult::Type UBTTask_MoveToPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyAI *CharPC = Cast<AEnemyAI>(OwnerComp.GetAIOwner());
	AVIMCharacter *Enemy = Cast<AVIMCharacter>(OwnerComp.GetBlackboardComponent()->GetValue<UBlackboardKeyType_Object>(CharPC->EnemyKeyID));

	if (Enemy)
	{
		CharPC->MoveToActor(Enemy, 5.f, true, true, true, 0, true);
		return EBTNodeResult::Succeeded;
	}
	else
	{
		return EBTNodeResult::Failed;
	}
	EBTNodeResult::Failed;
}

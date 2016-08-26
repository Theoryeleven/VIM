// copyright The Perfect Game Company 2016

#include "VIM.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"
#include "GameFramework/Controller.h"
#include "VIMMobb.h"
#include "VCharacter.h"
#include "EnemyAI.h"
#include "BTDecorator_CloseEnough.h"



UBTDecorator_CloseEnough::UBTDecorator_CloseEnough()
{
	NodeName = "Close Enough";
	bCreateNodeInstance = true;
	
}

bool UBTDecorator_CloseEnough::bCloseEnough(AEnemyAI *AI, AVCharacter *Player)const
{
	auto PlayerLocation = Player->GetActorLocation().ToString();
	auto AILocation = AI->GetPawn()->GetActorLocation().ToString();
	float MeasuredDistance = AI->GetPawn()->GetDistanceTo(Player);
	if (MeasuredDistance <= ApropriateDistance)
	{
		return false;
	}
	else
	{
		return true;
	}
	
}
bool UBTDecorator_CloseEnough::CalculateRawConditionValue(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory) const
{
	
	AEnemyAI *CharPC = Cast<AEnemyAI>(OwnerComp.GetAIOwner());
	AVCharacter *Enemy = Cast<AVCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
	bool bTooClose = bCloseEnough(CharPC, Enemy);
	return bTooClose;
}




// copyright The Perfect Game Company 2016

#include "VIM.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"
#include "VIMMobb.h"
#include "EnemyAI.h"

AEnemyAI::AEnemyAI()
{
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));

	BehaviourComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviourComp"));
}

void AEnemyAI::Possess(APawn * InPawn)
{
	Super::Possess(InPawn);
	AVIMMobb *Char = Cast<AVIMMobb>(InPawn);

	if (Char && Char->BotBehaviour)
	{
		BlackboardComp->InitializeBlackboard(*Char->BotBehaviour->BlackboardAsset);
		EnemyKeyID = BlackboardComp->GetKeyID("Target");

		BehaviourComp->StartTree(*Char->BotBehaviour);

	}
}

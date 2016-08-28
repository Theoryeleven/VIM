// copyright The Perfect Game Company 2016

#include "VIM.h"
#include "VAIController.h"
#include "VAICharacter.h"

/* AI Specific includes */
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

AVAIController::AVAIController(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BehaviorComp = ObjectInitializer.CreateDefaultSubobject<UBehaviorTreeComponent>(this, TEXT("BehaviorComp"));
	BlackboardComp = ObjectInitializer.CreateDefaultSubobject<UBlackboardComponent>(this, TEXT("BlackboardComp"));

	/* Match with the AIBlackboard */
	PatrolLocationKeyName = "PatrolLocation";
	CurrentWaypointKeyName = "CurrentWaypoint";
	BotTypeKeyName = "BotType";
	TargetEnemyKeyName = "TargetEnemy";

	/* Initializes PlayerState so we can assign a team index to AI */
	bWantsPlayerState = true;
}


void AVAIController::Possess(class APawn* InPawn)
{
	Super::Possess(InPawn);

	AVAICharacter* MeleeBot = Cast<AVAICharacter>(InPawn);
	if (MeleeBot)
	{
		if (MeleeBot->BehaviorTree->BlackboardAsset)
		{
			BlackboardComp->InitializeBlackboard(*MeleeBot->BehaviorTree->BlackboardAsset);

			/* Make sure the Blackboard has the type of bot we possessed */
			SetBlackboardBotType(MeleeBot->BotType);
		}

		BehaviorComp->StartTree(*MeleeBot->BehaviorTree);
	}
}


void AVAIController::UnPossess()
{
	Super::UnPossess();

	/* Stop any behavior running as we no longer have a pawn to control */
	BehaviorComp->StopTree();
}


void AVAIController::SetWaypoint(AVWayPoint* NewWaypoint)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsObject(CurrentWaypointKeyName, NewWaypoint);
	}
}


void AVAIController::SetTargetEnemy(APawn* NewTarget)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsObject(TargetEnemyKeyName, NewTarget);
	}
}


AVWayPoint* AVAIController::GetWaypoint()
{
	if (BlackboardComp)
	{
		return Cast<AVWayPoint>(BlackboardComp->GetValueAsObject(CurrentWaypointKeyName));
	}

	return nullptr;
}


AVBaseCharacter* AVAIController::GetTargetEnemy()
{
	if (BlackboardComp)
	{
		return Cast<AVBaseCharacter>(BlackboardComp->GetValueAsObject(TargetEnemyKeyName));
	}

	return nullptr;
}


void AVAIController::SetBlackboardBotType(EBotBehaviorType NewType)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsEnum(BotTypeKeyName, (uint8)NewType);
	}
}


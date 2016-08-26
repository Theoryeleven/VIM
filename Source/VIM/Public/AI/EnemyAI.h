// copyright The Perfect Game Company 2016

#pragma once

#include "AIController.h"
#include "EnemyAI.generated.h"

/**
 * 
 */
UCLASS()
class VIM_API AEnemyAI : public AAIController
{
	GENERATED_BODY()

	UPROPERTY(transient)
	class UBlackboardComponent *BlackboardComp;

	UPROPERTY(transient)
	class UBehaviorTreeComponent *BehaviourComp;

public:

	AEnemyAI();

	virtual void Possess(APawn* InPawn) override;

	uint8 EnemyKeyID;

	
	
};

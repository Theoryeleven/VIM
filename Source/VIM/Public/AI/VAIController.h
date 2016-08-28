// copyright The Perfect Game Company 2016

#pragma once

#include "AIController.h"
#include "VCharacter.h"
#include "VWayPoint.h"
#include "VAIController.generated.h"

/**
 * 
 */
UCLASS()
class VIM_API AVAIController : public AAIController
{
	GENERATED_BODY()
	
		AVAIController(const class FObjectInitializer& ObjectInitializer);

	/* Called whenever the controller possesses a character bot */
	virtual void Possess(class APawn* InPawn) override;

	virtual void UnPossess() override;

	UBehaviorTreeComponent* BehaviorComp;

	UBlackboardComponent* BlackboardComp;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
		FName TargetEnemyKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
		FName PatrolLocationKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
		FName CurrentWaypointKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
		FName BotTypeKeyName;

public:

	AVWayPoint* GetWaypoint();

	AVBaseCharacter* GetTargetEnemy();

	void SetWaypoint(AVWayPoint* NewWaypoint);

	void SetTargetEnemy(APawn* NewTarget);

	void SetBlackboardBotType(EBotBehaviorType NewType);

	/** Returns BehaviorComp subobject **/
	FORCEINLINE UBehaviorTreeComponent* GetBehaviorComp() const { return BehaviorComp; }

	FORCEINLINE UBlackboardComponent* GetBlackboardComp() const { return BlackboardComp; }
};
	
	


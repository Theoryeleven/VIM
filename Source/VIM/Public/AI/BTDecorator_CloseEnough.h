// copyright The Perfect Game Company 2016

#pragma once

#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "BTDecorator_CloseEnough.generated.h"

class AEnemyAI;
class AVCharacter;

/**
 * 
 */
UCLASS()
class VIM_API UBTDecorator_CloseEnough : public UBTDecorator_BlackboardBase
{
	GENERATED_BODY()
		UBTDecorator_CloseEnough();
	
protected:
	bool  bCloseEnough(AEnemyAI *AI, AVCharacter *Player)const;

	UPROPERTY(EditAnywhere, Category = "Setup")
		float ApropriateDistance = 100.f;

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const;
private:
};

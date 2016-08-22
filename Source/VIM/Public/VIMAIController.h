// copyright The Perfect Game Company 2016

#pragma once

#include "AIController.h"
#include "VIMAIController.generated.h"


UENUM()
enum class EAIType : uint8
{
	Ranged,
	Melee,
	Sneaky,
	Mage,
	Bomber
};
/**
 * Fully Functional AI class with multiple Subtypes
 */

UCLASS()
class VIM_API AVIMAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AVIMAIController();
protected:
	// Begin PlayerController interface
	virtual void Tick(float DeltaTime) override;

private:
	uint32 bMoveToPlayer : 1;

	/** Navigate AI to the current Player location. */
	void MoveToDestination();

	/** Navigate AI to the given world location. */
	void SetNewMoveDestination(const FVector DestLocation);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
		EAIType SetAIType = EAIType::Melee; //defaults to melee if no other ai is sellected
};

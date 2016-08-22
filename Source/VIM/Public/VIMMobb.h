// copyright The Perfect Game Company 2016

#pragma once

#include "GameFramework/Character.h"
#include "VIMMobb.generated.h"

UCLASS(Blueprintable)
class VIM_API AVIMMobb : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVIMMobb();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	
};

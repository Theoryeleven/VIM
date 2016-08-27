// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "VIM.h"
#include "VUsableActor.h"


AVUsableActor::AVUsableActor(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MeshComp = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Mesh"));
	RootComponent = MeshComp;
}


void AVUsableActor::OnUsed(APawn* InstigatorPawn)
{
	// Nothing to do here...
}


void AVUsableActor::OnBeginFocus()
{
	// Used by custom PostProcess to render outlines
	MeshComp->SetRenderCustomDepth(true);
}


void AVUsableActor::OnEndFocus()
{
	// Used by custom PostProcess to render outlines
	MeshComp->SetRenderCustomDepth(false);
}

#include "VIM.h"
#include "VHUD.h"
#include "VCharacter.h"
#include "VUsableActor.h"
#include "VPlayerController.h"


AVHUD::AVHUD(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	/* You can use the FObjectFinder in C++ to reference content directly in code. Although it's advisable to avoid this and instead assign content through Blueprint child classes.
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDCenterDotObj(TEXT("/Game/UI/HUD/T_CenterDot_M.T_CenterDot_M"));
	CenterDotIcon = UCanvas::MakeIcon(HUDCenterDotObj.Object);*/
}


void AVHUD::DrawHUD()
{
	Super::DrawHUD();

	
}





void AVHUD::OnStateChanged_Implementation(EHUDState NewState)
{
	CurrentState = NewState;
}


EHUDState AVHUD::GetCurrentState()
{
	return CurrentState;
}

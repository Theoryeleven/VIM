// copyright The Perfect Game Company 2016

#include "VIM.h"
#include "VPlayerState.h"
#include "VGameState.h"

AVPlayerState::AVPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	/* AI will remain in team 0, players are updated to team 1 through the GameMode::InitNewPlayer */
	TeamNumber = 0;
}


void AVPlayerState::Reset()
{
	Super::Reset();

	NumKills = 0;
	NumDeaths = 0;
	Score = 0;
}

void AVPlayerState::AddKill()
{
	NumKills++;
}

void AVPlayerState::AddDeath()
{
	NumDeaths++;
}

void AVPlayerState::ScorePoints(int32 Points)
{
	Score += Points;

	/* Add the score to the global score count */
	AVGameState* GS = Cast<AVGameState>(GetWorld()->GameState);
	if (GS)
	{
		GS->AddScore(Points);
	}
}


void AVPlayerState::SetTeamNumber(int32 NewTeamNumber)
{
	TeamNumber = NewTeamNumber;
}


int32 AVPlayerState::GetTeamNumber() const
{
	return TeamNumber;
}

int32 AVPlayerState::GetKills() const
{
	return NumKills;
}

int32 AVPlayerState::GetDeaths() const
{
	return NumDeaths;
}


float AVPlayerState::GetScore() const
{
	return Score;
}


void AVPlayerState::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}

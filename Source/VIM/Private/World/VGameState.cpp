// copyright The Perfect Game Company 2016

#include "VIM.h"
#include "VPlayerController.h"
#include "VGameState.h"

AVGameState::AVGameState(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	/* 1 SECOND real time is 1*TimeScale MINUTES game time */
	TimeScale = 8.0f;
	bIsNight = false;

	SunriseTimeMark = 6.0f;
	SunsetTimeMark = 18.0f;
}


void AVGameState::SetTimeOfDay(float NewTimeOfDay)
{
	ElapsedGameMinutes = NewTimeOfDay;
}


bool AVGameState::GetIsNight()
{
	return bIsNight;
}


float AVGameState::GetTimeOfDayIncrement()
{
	return (GetWorldSettings()->GetEffectiveTimeDilation() * TimeScale);
}


int32 AVGameState::GetElapsedDays()
{
	const float MinutesInDay = 24 * 60;
	const float ElapsedDays = ElapsedGameMinutes / MinutesInDay;
	return FMath::FloorToInt(ElapsedDays);
}


int32 AVGameState::GetElapsedFullDaysInMinutes()
{
	const int32 MinutesInDay = 24 * 60;
	return GetElapsedDays() * MinutesInDay;
}


bool AVGameState::GetAndUpdateIsNight()
{
	const float TimeOfDay = ElapsedGameMinutes - GetElapsedFullDaysInMinutes();
	if (TimeOfDay > (SunriseTimeMark * 60) && TimeOfDay < (SunsetTimeMark * 60))
	{
		bIsNight = false;
	}
	else
	{
		bIsNight = true;
	}

	return bIsNight;
}


int32 AVGameState::GetRealSecondsTillSunrise()
{
	float SunRiseMinutes = (SunriseTimeMark * 60);
	const int32 MinutesInDay = 24 * 60;

	float ElapsedTimeToday = GetElapsedMinutesCurrentDay();
	if (ElapsedTimeToday < SunRiseMinutes)
	{
		/* Still early in day cycle, so easy to get remaining time */
		return (SunRiseMinutes - ElapsedTimeToday) / TimeScale;
	}
	else
	{
		/* Sunrise will happen "tomorrow" so we need to add another full day to get remaining time */
		float MaxTimeTillNextSunrise = MinutesInDay + SunRiseMinutes;
		return (MaxTimeTillNextSunrise - ElapsedTimeToday) / TimeScale;
	}
}


int32 AVGameState::GetElapsedMinutesCurrentDay()
{
	return ElapsedGameMinutes - GetElapsedFullDaysInMinutes();
}




int32 AVGameState::GetTotalScore()
{
	return TotalScore;
}


void AVGameState::AddScore(int32 Score)
{
	TotalScore += Score;
}


void AVGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(AVGameState, ElapsedGameMinutes);
	DOREPLIFETIME(AVGameState, bIsNight);
	DOREPLIFETIME(AVGameState, TotalScore);
}



#pragma once
#include "Timeline.h"

Timeline::Timeline() :
	duration(0), loops(true)
{
	currentTime = 0.0f;
}

Timeline::Timeline(float duration, bool loops) :
	duration(duration), loops(loops)
{
	currentTime = 0.0f;
}

/// <summary>
/// Update the current time by delta amount.
/// Clamps or repeats based on settings 
/// </summary>
/// <param name="delta"></param>
void Timeline::UpdateTimeline(float delta)
{
	currentTime += delta;

	if (currentTime <= duration)
		return;
	
	// We can assume it's now greater that duration 

	if (loops)
	{
		currentTime = 0.0f;
	}
	else
	{
		currentTime = duration;
	}
}

void Timeline::SetTime(float time)
{
	currentTime = time;
}
void Timeline::SetLoops(bool loops)
{
	this->loops = loops;
}

void Timeline::SetDuration(float duration)
{
	this->duration = duration;
}

float Timeline::GetTime()
{
	return currentTime;
}

float Timeline::GetUnitInterval()
{
	return currentTime / duration;
}

bool Timeline::GetLoops()
{
	return loops;
}
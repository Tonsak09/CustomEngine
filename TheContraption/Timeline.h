#pragma once

/*
	This is a helpful object that lets us
	keep better track of time and get the
	desired details of the time for us.
	
	This data structure is used by multiple
	different classes so it should have its
	own file 
*/


struct Timeline
{
public:
	Timeline();
	Timeline(float duration, bool loops = true);

	void UpdateTimeline(float delta);

	void SetTime(float time);
	void SetLoops(bool loops);
	void SetDuration(float time);

	float GetTime();
	float GetUnitInterval();
	bool GetLoops();

private:
	float currentTime;

	float duration;
	bool loops;
};
#pragma once

#include <memory>

#include "AnimClip.h"
#include "SkeletonHierarchy.h"

/*
	The purpose of the animator is to take the information
	from the current clip and apply its data to the skeletal
	hierarchy. 

	Though internally the animator also has a timeline it 
	cannot be manipulated like individual clips. Playback
	manipulation MUST happen at the clip level. 
*/

class Animator
{
public:
	Animator(std::shared_ptr<SkeletalHierarchy> skeleton);
	Animator(std::shared_ptr<SkeletalHierarchy> hierarchy, std::shared_ptr<AnimClip> clip);

	void AnimateSkeleton(float delta);

	void SetClip(std::shared_ptr<AnimClip> clip);

private:
	std::shared_ptr<SkeletalHierarchy> skeleton;
	std::shared_ptr<AnimClip> currentClip;

	std::shared_ptr<Timeline> timeLine;
};
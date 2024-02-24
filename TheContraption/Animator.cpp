#pragma once
#include "Animator.h"

Animator::Animator(std::shared_ptr<SkeletalHierarchy> skeleton) :
	skeleton(skeleton) 
{ 
	timeLine = std::make_shared<Timeline>();
}

Animator::Animator(std::shared_ptr<SkeletalHierarchy> hierarchy, std::shared_ptr<AnimClip> clip) :
	skeleton(hierarchy)
{
	timeLine = std::make_shared<Timeline>(clip->GetDuration(), clip->GetLoops());
	SetClip(clip); // Set clip here not in >....<
}

/// <summary>
/// Updates timeline with delta and then applies bone changes
/// to the attached skeletal hierarchy 
/// </summary>
/// <param name="delta"></param>
void Animator::AnimateSkeleton(float delta)
{

}

void Animator::SetClip(std::shared_ptr<AnimClip> clip)
{
	// Reset timeline 
	timeLine->SetTime(0.0f);

	this->currentClip = clip;
	timeLine->SetDuration(clip->GetDuration());
}
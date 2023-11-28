#include "BasicAnimation.h"
using namespace DirectX;

#pragma region MANAGER

BasicAnimationManager::BasicAnimationManager()
{

}

BasicAnimationManager::~BasicAnimationManager()
{

}

void BasicAnimationManager::AddAnimation(std::shared_ptr<Transform> target, DirectX::XMFLOAT3 start, DirectX::XMFLOAT3 end, float time, int curveType)
{
	animations[target] = std::make_shared<AnimDetails>(
		target,
		start,
		end,
		time,
		curveType);
}

void BasicAnimationManager::UpdateAnimations(float deltaTime)
{
	// Iterate through animations 
	for (auto anim = animations.begin(); anim != animations.end(); ++anim) {
		
		if (anim->second->animFinished)
		{
			// Animation is no longer in use 
			animations.erase(anim);
			return;
		}

		anim->second->UpdateAnimation(deltaTime);
	}
}

bool BasicAnimationManager::IsRunningAnimations()
{
	return animations.size() != 0;
}

#pragma endregion

AnimDetails::AnimDetails(std::shared_ptr<Transform> target, DirectX::XMFLOAT3 start, DirectX::XMFLOAT3 end, float time, int curveType):
	target(target), start(start), end(end), totalTime(time), curveType(curveType)
{
	timer = 0.0f;
	animFinished = false;
}

void AnimDetails::UpdateAnimation(float deltaTime)
{
	if (timer > totalTime)
	{
		animFinished = true;
		return;
	}

	DirectX::XMFLOAT3 current;
	DirectX::XMStoreFloat3(
		&current,
		DirectX::XMLoadFloat3(&start) + (DirectX::XMLoadFloat3(&end) - DirectX::XMLoadFloat3(&start)) * GetCurveByIndex(curveType, timer / totalTime));	// Unclamped Lerp 
	target->SetPosition(current);

	timer += deltaTime;
}



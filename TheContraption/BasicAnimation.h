#pragma once
#include <memory>
#include <DirectXMath.h>
#include <vector>
#include <unordered_map>

#include "Transform.h"
#include "AnimCurves.h"

/*
	Purpose of this file is to allow simple animation
	translation controls for the user by managing it 
	for us here
*/



/// <summary>
/// Used to manage translation data
/// </summary>
struct AnimDetails
{
public:
	AnimDetails(std::shared_ptr<Transform> target, DirectX::XMFLOAT3 start, DirectX::XMFLOAT3 end, float time, int curveType);
	void UpdateAnimation(float deltaTime);

	bool animFinished;
private:
	std::shared_ptr<Transform> target;
	DirectX::XMFLOAT3 start;
	DirectX::XMFLOAT3 end;

	// Type of animation curve which is used in the AnimCurves file 
	int curveType;

	// How much times has passed since starting this animation 
	float timer;
	// Total amount of time for animtion 
	float totalTime;
};


class BasicAnimationManager
{
public:
	BasicAnimationManager();
	~BasicAnimationManager();

	void AddAnimation(std::shared_ptr<Transform> target, DirectX::XMFLOAT3 start, DirectX::XMFLOAT3 end, float time, int curveType);
	void UpdateAnimations(float deltaTime);

	//void RemoveAnimation(std::shared_ptr<DirectX::XMFLOAT3> target);

	/*
		Possible future functions:
		StopAnimation(int index)
		StopAnimation(target)
	*/
private:
	/// <summary>
	/// Hold all currently active animation 
	/// </summary>
	std::unordered_map <std::shared_ptr<Transform>, std::shared_ptr<AnimDetails>> animations;
};


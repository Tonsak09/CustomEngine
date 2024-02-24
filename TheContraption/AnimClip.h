#pragma once

#include <DirectXMath.h>
#include <memory>
#include <vector>
#include <unordered_map>

#include <string>

#include "AnimMoment.h"
#include "Timeline.h"

/*
	This holds the the series of transforms for each bone 
	during the animation and a timeline to go with it. There
	are also settings that allow the manipulation of the 
	retrieval of moments like time scale. 
*/


/// <summary>
/// Used to store the history of a single bone during an
/// animation clip 
/// </summary>
struct BoneClip
{
public:
	BoneClip(
		std::string name,
		std::vector<float> positionKeyTimes,
		std::vector<DirectX::XMFLOAT3> positions,
		std::vector<float> rotationKeyTimes,
		std::vector<DirectX::XMFLOAT4> rotations);

	std::string GetName();
	std::shared_ptr<AnimMoment> GetMoment(float time);

private:
	std::string name;

	std::vector<float> positionKeyTimes;
	std::vector<DirectX::XMFLOAT3> positions;

	std::vector<float> rotationKeyTimes;
	std::vector<DirectX::XMFLOAT4> rotations;
};


class AnimClip
{
public:
	AnimClip(std::vector<std::shared_ptr<BoneClip>> boneClips, float duration, float ticksPerSecond, bool loops = true);

	void AddBone(std::shared_ptr<BoneClip> boneClip);
	std::vector<std::shared_ptr<AnimMoment>> GetMoments(float time);

	float GetDuration();
	bool GetLoops();

private:

	/*
		To retrive a keyframe we need to find out which frames
		the given time is between and then interpolate between
		them. 

		Pattern is as follows:
		1) Make sure time is within range 
			-> If out of range clamp to either max or min 
		2) Search for keyframe before and after
		3) Get interpolated values 
		4) Create anim-moment from values and return 

		The key time indicies match up with the indicies of the
		desired value vector 
	*/

	std::vector<std::shared_ptr<BoneClip>> boneClips;
	std::unordered_map<std::string, std::shared_ptr<BoneClip>> nameToBoneClip;

	float duration; // duration in ticks 
	float ticksPerSecond;
	bool loops;
};
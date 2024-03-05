#pragma once

#include "AnimClip.h"


AnimClip::AnimClip(std::vector<std::shared_ptr<BoneClip>> boneClips, float duration, float ticksPerSecond, bool loops) :
	boneClips(boneClips), duration(duration), ticksPerSecond(ticksPerSecond), loops(loops)
{
	// Setup unorder list for later use 
	for (auto clip : boneClips)
	{
		nameToBoneClip[clip->GetName()] = clip;
	}
}

/// <summary>
/// Retrieve the specific transforms of each bone in between, or on,
/// frames along with the bone's name. 
/// </summary>
/// <param name="time"></param>
/// <returns></returns>
std::vector<std::shared_ptr<AnimMoment>> AnimClip::GetMoments(float time)
{
	std::vector<std::shared_ptr<AnimMoment>> moments;

	for (auto clip : boneClips)
	{
		moments.push_back(clip->GetMoment(time));
	}

	return moments;
}

/// <summary>
/// Add another bone to the clip to be animated 
/// </summary>
/// <param name="time"></param>
/// <returns></returns>
void AnimClip::AddBone(std::shared_ptr<BoneClip> boneClip)
{
	boneClips.push_back(boneClip);
	nameToBoneClip[boneClip->GetName()] = boneClip;
}

/// <summary>
/// Get the length of time in ticks that this clip lasts 
/// </summary>
/// <returns></returns>
float AnimClip::GetDuration()
{
	return duration;
}

/// <summary>
/// Whether or not this animation loops 
/// </summary>
/// <returns></returns>
bool AnimClip::GetLoops()
{
	return loops;
}


BoneClip::BoneClip(std::string name,
	std::vector<float> positionKeyTimes,
	std::vector<DirectX::XMFLOAT3> positions,
	std::vector<float> rotationKeyTimes,
	std::vector<DirectX::XMFLOAT4> rotations) :
	name(name), 
	positionKeyTimes(positionKeyTimes), 
	positions(positions), 
	rotationKeyTimes(rotationKeyTimes), 
	rotations(rotations) { }

std::string BoneClip::GetName()
{
	return name;
}

/// <summary>
/// Get the position and rotation of this bone at a
/// given point of time. Interpolates between frames 
/// </summary>
/// <param name="time"></param>
/// <returns></returns>
std::shared_ptr<AnimMoment> BoneClip::GetMoment(float time)
{
	std::shared_ptr<AnimMoment> moment;

	// We clamp the values 
	// If out of range use the appropriate edge case 
	if (time <= 0.0f)
	{
		moment = std::make_shared<AnimMoment>(name, positions[0], rotations[0]);
	}
	else if (time >= positionKeyTimes[positionKeyTimes.size() - 1])
	{
		int endIndex = positionKeyTimes.size() - 1;
		moment = std::make_shared<AnimMoment>(name, positions[endIndex], rotations[endIndex]);
	}
	else
	{
		int tailPosIndex = 0;
		int headPosIndex = 0;
		
		int tailRotIndex = 0;
		int headRotIndex = 0;

		// Find position key times 
		for (int i = 0; i < positionKeyTimes.size(); i++)
		{
			if (positionKeyTimes[i] > time )
			{
				// Tail found!
				tailPosIndex = i - 1;
				headPosIndex = (i); //+ 1 >= positionKeyTimes.size()) ? i : i + 1;

				break;
			}
		}

		// Find rotation key times 
		for (int i = 0; i < rotationKeyTimes.size(); i++)
		{
			if (rotationKeyTimes[i] >= time)
			{
				// Tail found!
				tailRotIndex = i;
				headRotIndex = (i + 1 >= rotationKeyTimes.size()) ? i : i + 1;

				break;
			}
		}


		// Lerp Position
		DirectX::XMFLOAT3 pos;
		float lerpPos;

		if (tailPosIndex == headPosIndex)
		{
			lerpPos = 0.0f;
		}
		else
		{
			lerpPos = (time - positionKeyTimes[tailPosIndex]) /
				(positionKeyTimes[headPosIndex] - positionKeyTimes[tailPosIndex]);
		}

		DirectX::XMVECTOR tailPos = DirectX::XMLoadFloat3(&positions[tailPosIndex]);
		DirectX::XMVECTOR headPos = DirectX::XMLoadFloat3(&positions[headPosIndex]);

		DirectX::XMStoreFloat3(&pos, DirectX::XMVectorLerp(tailPos, headPos, lerpPos));

		// Slerp Rotation 
		DirectX::XMFLOAT4 rot;
		float lerpRot = (time - rotationKeyTimes[tailRotIndex]) /
			(rotationKeyTimes[headRotIndex] - rotationKeyTimes[tailRotIndex]);
		DirectX::XMVECTOR tailRot = DirectX::XMLoadFloat4(&rotations[tailRotIndex]);
		DirectX::XMVECTOR headRot = DirectX::XMLoadFloat4(&rotations[headRotIndex]);

		DirectX::XMStoreFloat4(&rot, DirectX::XMQuaternionSlerp(tailRot, headRot, lerpRot));

		moment = std::make_shared<AnimMoment>(name, pos, rot);
	}

	return moment;
}
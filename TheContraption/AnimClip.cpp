#pragma once

#include "AnimClip.h"


AnimClip::AnimClip(std::vector<std::shared_ptr<BoneClip>> boneClips, float duration, float ticksPerSecond) :
	boneClips(boneClips), duration(duration), ticksPerSecond(ticksPerSecond)
{
	// Setup unorder list for later use 
	for (auto clip : boneClips)
	{
		nameToBoneClip[clip->GetName()] = clip;
	}
}

std::vector<std::shared_ptr<AnimMoment>> AnimClip::GetMoments(float time)
{
	std::vector<std::shared_ptr<AnimMoment>> moments;

	for (auto clip : boneClips)
	{
		moments.push_back(clip->GetMoment(time));
	}

	return moments;
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
			if (positionKeyTimes[i] <= time)
			{
				// Tail found!
				tailPosIndex = i;
				headPosIndex = (i + 1 >= positionKeyTimes.size()) ? i : i + 1;

				break;
			}
		}

		// Find rotation key times 
		for (int i = 0; i < rotationKeyTimes.size(); i++)
		{
			if (rotationKeyTimes[i] <= time)
			{
				// Tail found!
				tailPosIndex = i;
				headPosIndex = (i + 1 >= rotationKeyTimes.size()) ? i : i + 1;

				break;
			}
		}


		// Lerp Position
		DirectX::XMFLOAT3 pos;
		float lerpPos = (time - positionKeyTimes[tailPosIndex]) / 
						(positionKeyTimes[headPosIndex] - positionKeyTimes[tailPosIndex]);
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
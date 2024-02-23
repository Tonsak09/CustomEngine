#pragma once

#include "AnimMoment.h"

AnimMoment::AnimMoment(std::string name, DirectX::XMFLOAT3 position, DirectX::XMFLOAT4 rotation) :
	name(name), position(position), rotation(rotation) { }

std::string AnimMoment::GetName()
{
	return name;
}

DirectX::XMFLOAT3 AnimMoment::GetPosition()
{
	return position;
}

DirectX::XMFLOAT4 AnimMoment::GetRotation()
{
	return rotation;
}
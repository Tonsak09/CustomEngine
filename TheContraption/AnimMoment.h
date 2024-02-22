#pragma once
#include <DirectXMath.h>
#include <string>

/*
	The purpose of the anim moment is to hold onto a bone's
	transform and details at a given moment of time. 
*/

struct AnimMoment
{
public:
	AnimMoment(std::string name, DirectX::XMFLOAT3 position, DirectX::XMFLOAT4 rotation);

	std::string GetName();
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT4 GetRotation();

private:
	std::string name;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 rotation;
};
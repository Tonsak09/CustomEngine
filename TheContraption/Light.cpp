#include "Lights.h"

using namespace DirectX;

//DirectX::XMMATRIX Light::GetlightView()
//{
//	if (!matDirty)
//		return lightView;
//
//	lightView = DirectX::XMMatrixLookToLH(
//		-DirectX::XMLoadFloat3(&directiton) * 20, // Position: "Backing up" 20 units from origin
//		DirectX::XMLoadFloat3(&directiton), // Direction: light's direction
//		DirectX::XMVectorSet(0, 1, 0, 0)); // Up: World up vector (Y axis)
//}
#pragma once
#include <DirectXMath.h>
using namespace DirectX;

struct ShadadowShaderData
{
public:
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV;

	XMFLOAT4X4  world;
	XMFLOAT4X4  view;
	XMFLOAT4X4  projection;
};
#pragma once
#include <DirectXMath.h>
using namespace DirectX;

struct ShadadowShaderData
{
public:
	XMMATRIX  world;
	XMMATRIX  view;
	XMMATRIX  projection;
};
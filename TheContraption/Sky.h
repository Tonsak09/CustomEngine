#pragma once
#include <DirectXMath.h>
#include <memory>
#include <wrl/client.h>
#include <d3d11.h>
#include "Mesh.h"
#include "SimpleShader.h"
#include "PathHelpers.h"
#include "Camera.h"
class Sky
{
public:
	Sky(
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler,
		std::shared_ptr<Mesh> mesh,
		const wchar_t right[],
		const wchar_t left[],
		const wchar_t up[],
		const wchar_t down[],
		const wchar_t front[],
		const wchar_t back[],
		const wchar_t pixelShaderPath[] = L"SkyPixelShader.cso",
		const wchar_t vertexShaderPath[] = L"SkyVertexShader.cso"
	);

	void Draw(std::shared_ptr<Camera> cam);
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetCubeSRV();
private:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubeSRV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> stencilState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizeState;

	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<SimplePixelShader> skyPS;
	std::shared_ptr<SimpleVertexShader> skyVS;



	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back);
};
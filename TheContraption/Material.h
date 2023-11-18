#pragma once
#include <DirectXMath.h>
#include <memory>

#include "SimpleShader.h"
#include <unordered_map>

class Material
{
public:
	Material(DirectX::XMFLOAT4 tint, float roughness, DirectX::XMFLOAT2 uvOffset, std::shared_ptr<SimpleVertexShader> vertex, std::shared_ptr<SimplePixelShader> pixel);

	/// <summary>
	/// Get this material's current color tint 
	/// </summary>
	/// <returns></returns>
	DirectX::XMFLOAT4 GetTint();
	/// <summary>
	/// Get this material's current rougness 
	/// </summary>
	/// <returns></returns>
	float GetRoughness();
	/// <summary>
	/// Get this material's current vertex shader shared pointer 
	/// </summary>
	/// <returns></returns>
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	/// <summary>
	/// Get this material's current pixel shader shared pointer 
	/// </summary>
	/// <returns></returns>
	std::shared_ptr<SimplePixelShader> GetPixelShader();

	/// <summary>
	/// Get this material's current uv offset 
	/// </summary>
	/// <returns></returns>
	DirectX::XMFLOAT2 GetUVOffset();

	/// <summary>
	/// Set this material's current color tint 
	/// </summary>
	/// <param name="nextTint"></param>
	void SetTint(DirectX::XMFLOAT4 nextTint);

	/// <summary>
	/// Set this material's current roughness 
	/// </summary>
	/// <param name="nextRoughness"></param>
	void SetRoughness(float nextRoughness);

	/// <summary>
	/// Set this material's current uv offset 
	/// </summary>
	/// <param name="nextOffset"></param>
	void SetUVOffset(DirectX::XMFLOAT2 nextOffset);

	/// <summary>
	/// Set this material's current vertex shader 
	/// </summary>
	/// <param name="nextVertex"></param>
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> nextVertex);

	/// <summary>
	/// Set this materials current pixel shader 
	/// </summary>
	/// <param name="nextPixel"></param>
	void SetPixelShader(std::shared_ptr<SimplePixelShader> nextPixel);

	void AddTextureSRV(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddSampler(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);

	void PrepareMaterial();

private:
	DirectX::XMFLOAT4 tint;
	DirectX::XMFLOAT3 camPos;
	float roughness;
	DirectX::XMFLOAT2 uvOffset;

	std::shared_ptr<SimpleVertexShader> vertex;
	std::shared_ptr<SimplePixelShader> pixel;

	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;
};


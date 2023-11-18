#include "Material.h"

Material::Material(DirectX::XMFLOAT4 tint, float roughness, DirectX::XMFLOAT2 uvOffset, std::shared_ptr<SimpleVertexShader> vertex, std::shared_ptr<SimplePixelShader> pixel) :
	tint(tint), camPos(camPos), roughness(roughness), uvOffset(uvOffset), vertex(vertex), pixel(pixel) {}

DirectX::XMFLOAT4 Material::GetTint()
{
	return tint;
}

float Material::GetRoughness()
{
	return roughness;
}

std::shared_ptr<SimpleVertexShader> Material::GetVertexShader()
{
	return vertex;
}

std::shared_ptr<SimplePixelShader> Material::GetPixelShader()
{
	return pixel;
}

DirectX::XMFLOAT2 Material::GetUVOffset()
{
	return uvOffset;
}



void Material::SetTint(DirectX::XMFLOAT4 nextTint)
{
	tint = nextTint;
}

void Material::SetRoughness(float nextRoughness)
{
	roughness = nextRoughness;
}

void Material::SetUVOffset(DirectX::XMFLOAT2 nextOffset)
{
	uvOffset = nextOffset;
}

void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> nextVertex)
{
	vertex = nextVertex;
}

void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> nextPixel)
{
	pixel = nextPixel;
}

void Material::AddTextureSRV(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	textureSRVs.insert({ name, srv });
}

void Material::AddSampler(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
	samplers.insert({ name, sampler });
}

void Material::PrepareMaterial()
{
	for (auto& t : textureSRVs) { pixel->SetShaderResourceView(t.first.c_str(), t.second);	}
	for (auto& s : samplers	  )	{ pixel->SetSamplerState(s.first.c_str(), s.second);		}

}
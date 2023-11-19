#pragma once

struct MatData
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> albedo;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> spec;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughness;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metal;

	MatData(Microsoft::WRL::ComPtr<ID3D11Device> device)
	{
		
	}
};
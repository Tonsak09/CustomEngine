#pragma once


#include <d3d11.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include "Vertex.h"

#include <fstream>
#include <vector>
#include <DirectXMath.h>

class Mesh
{
private:
	void ContructVIBuffers(Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext, Vertex vertices[], unsigned int indices[]);

	// Buffers that connect data to the GPU 
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	Microsoft::WRL::ComPtr<ID3D11Device> device; 
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;

	int indicesCount;
	int vertexCount;

	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);

public:
	/// <summary>
	/// Create a mesh based on manually given vertex data
	/// </summary>
	Mesh(Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext, Vertex vertices[], unsigned int indices[], int vertexCount, int indexCount);
	/// <summary>
	/// Create a mesh based on a given obj file 
	/// </summary>
	Mesh(Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext, const wchar_t* file);
	~Mesh();

	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	int GetIndexCount();

	void Draw();
};


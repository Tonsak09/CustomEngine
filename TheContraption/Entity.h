#pragma once
#include <memory>
#include <DirectXMath.h>

#include "Transform.h"
#include "Mesh.h"
#include "Camera.h"

#include "Material.h"


class Entity
{
private:
	Transform transform;
	std::shared_ptr<Mesh> model;
	std::shared_ptr<Material> mat;
	
public:
	Entity(std::shared_ptr<Mesh> model, std::shared_ptr<Material> mat);

	std::shared_ptr<Mesh> GetModel();
	Transform* GetTransform();
	std::shared_ptr<Material> GetMat();
	void SetMat(std::shared_ptr<Material> nextMat);

	// In the future this could be allocated to a rendering class that holds all drawing data intstead
	// of objects drawing themselves 
	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, std::shared_ptr<Camera>);
	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, std::shared_ptr<Camera>, float time); // FOR NOISE DEMO 
};


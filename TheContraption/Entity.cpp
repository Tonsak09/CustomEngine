#include "Entity.h"

#include <time.h> // TEMPORARY FOR NOISE

Entity::Entity(std::shared_ptr<Mesh> model, std::shared_ptr<Material> mat) :
	model(model), mat(mat)
{
	transform = Transform();
}

std::shared_ptr<Mesh> Entity::GetModel()
{
	return model;
}

Transform* Entity::GetTransform() 
{ 
	return &transform; 
}

std::shared_ptr<Material> Entity::GetMat()
{
	return mat;
}

void Entity::SetMat(std::shared_ptr<Material> nextMat)
{
	mat = nextMat;
}

void Entity::Draw(
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, 
	std::shared_ptr<Camera> camera)
{
	mat->GetVertexShader()->SetShader();
	mat->GetPixelShader()->SetShader();


	std::shared_ptr<SimpleVertexShader> vs = mat->GetVertexShader();
	vs->SetFloat4("colorTint", mat->GetTint()); // Strings here MUST
	vs->SetMatrix4x4("world", transform.GetWorldMatrix()); // match variable
	vs->SetMatrix4x4("viewMatrix", *camera->GetViewMatrix().get()); // names in your
	vs->SetMatrix4x4("projMatrix", *camera->GetProjMatrix().get()); // shader’s cbuffer!
	vs->SetMatrix4x4("worldInvTranspose", transform.GetWorldInverseTransposeMatrix());

	vs->CopyAllBufferData();

	std::shared_ptr<SimplePixelShader> ps = mat->GetPixelShader();
	ps->SetFloat4("colorTint", mat->GetTint());

	//Transform* trans = camera->GetTransform();
	//DirectX::XMFLOAT3 pos = trans->GetPosition();
	ps->SetFloat3("camPos", *(camera->GetTransform()->GetPosition().get()));
	ps->SetFloat("roughness", mat->GetRoughness());
	ps->SetFloat2("uvOffset", mat->GetUVOffset());

	ps->CopyAllBufferData();

	mat->PrepareMaterial();

	model->Draw();
}

void Entity::Draw(
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, 
	std::shared_ptr<Camera> camera, float time)
{
	mat->GetVertexShader()->SetShader();
	mat->GetPixelShader()->SetShader();


	std::shared_ptr<SimpleVertexShader> vs = mat->GetVertexShader();
	//vs->SetFloat4("colorTint", mat->GetTint()); // Strings here MUST
	vs->SetMatrix4x4("world", transform.GetWorldMatrix()); // match variable
	vs->SetMatrix4x4("viewMatrix", *camera->GetViewMatrix().get()); // names in your
	vs->SetMatrix4x4("projMatrix", *camera->GetProjMatrix().get()); // shader’s cbuffer!

	vs->CopyAllBufferData();

	std::shared_ptr<SimplePixelShader> ps = mat->GetPixelShader();
	ps->SetFloat4("colorTint", mat->GetTint());
	ps->SetFloat("time", time); // Only passes if time is a variable 

	ps->CopyAllBufferData();

	model->Draw();
}

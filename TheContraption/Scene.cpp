#include "Scenes.h"

Scene::Scene(
	std::string sceneTitle,
	std::vector<std::shared_ptr<Camera>> cameras,
	std::vector<std::shared_ptr<Entity>> entities,
	std::vector<std::tuple<std::shared_ptr<Light>,std::shared_ptr<Entity>>> lightAndGui,
	std::shared_ptr<Sky> sky
) :
	sceneTitle(sceneTitle), cameras(cameras), entities(entities), lights(lights)
{
	// Start of cameras vector 
	currentCam = 0;

	// Split lights and their gui into two different vectors 
	SetLightsAndGui(lightAndGui);
}

Scene::Scene(std::string sceneTitle) :
	sceneTitle(sceneTitle)
{
	currentCam = 0;

	lightToGizmos = std::unordered_map<Light*, Entity*>();
	lightToShadowData = std::unordered_map<Light*, std::shared_ptr<ShadadowShaderData>>();

}

void Scene::DrawShadows(
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, 
	Microsoft::WRL::ComPtr<ID3D11Device> device, 
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufferRTV,
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthBufferDSV,
	std::shared_ptr<SimpleVertexShader> shadowVS, 
	int shadowMapResolution,
	float windowWidth, float windowHeight)
{

	// Acne fix 
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterizer;
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Min. precision units, not world units!
	shadowRastDesc.SlopeScaledDepthBias = 1.0f; // Bias more based on slope
	device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);

	// Set bias active 
	context->RSSetState(shadowRasterizer.Get());


	// Clear shadowmap
	context->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Setup output merger
	ID3D11RenderTargetView* nullRTV{};
	context->OMSetRenderTargets(1, &nullRTV, shadowDSV.Get());

	// Deactivate pixel shader 
	context->PSSetShader(0, 0, 0);

	// Change viewport 
	D3D11_VIEWPORT viewport = {};
	viewport.Width = (float)shadowMapResolution;
	viewport.Height = (float)shadowMapResolution;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	// TODO ---------------------------------------------------------------------------------- FIND WAY TO CHOOSE DIRECTIONAL LIGHT THAT WORKS AND NOT JUST 0
	// Entity Render Loop
	shadowVS->SetShader();
	shadowVS->SetMatrix4x4("view", lightToShadowData[lights[0].get()]->view);
	shadowVS->SetMatrix4x4("projection", lightToShadowData[lights[0].get()]->projection);
	// Loop and draw all entities
	for (auto& e : entities)
	{
		shadowVS->SetMatrix4x4("world", e->GetTransform()->GetWorldMatrix());
		shadowVS->CopyAllBufferData();
		e->GetModel()->Draw();
	}

	// Reset pipeline
	viewport.Width = windowWidth;
	viewport.Height = windowHeight;
	context->RSSetViewports(1, &viewport);
	context->OMSetRenderTargets(
		1,
		backBufferRTV.GetAddressOf(),
		depthBufferDSV.Get());


	// Reset bias 
	context->RSSetState(0);
}

void Scene::DrawEntities(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	for (unsigned int i = 0; i < entities.size(); i++)
	{
		DirectX::XMFLOAT3 ambient(0.1f, 0.1f, 0.25f);
		entities[i]->GetMat()->GetPixelShader()->SetFloat3("ambient", ambient);
		
		


		// For point light 
		float distance = 0;
		
		int dLights = 1;
		int sLights = 1;
		int pLights = 1;
		for (int l = 0; l < lights.size(); l++)
		{
			Light* light = lights[l].get();
			std::string name; 

			switch (light->type)
			{
			case LIGHT_TYPE_DIRECTIONAL:
				name = "directionalLight" + std::to_string(dLights);

				// Shadow setting 
				if (dLights == 1) // TODO CHANGE TO BE MORE DYNAMIC
				{
					entities[i]->GetMat()->GetVertexShader()->SetMatrix4x4("lightView", lightToShadowData[light]->view);
					entities[i]->GetMat()->GetVertexShader()->SetMatrix4x4("lightProjection", lightToShadowData[light]->projection);
					entities[i]->GetMat()->AddTextureSRV("ShadowMap", shadowSRV);
				}

				dLights++;
				break;
			case  LIGHT_TYPE_POINT:
				DirectX::XMVECTOR vector1 = DirectX::XMLoadFloat3(&light->position);
				DirectX::XMVECTOR vector2 = DirectX::XMLoadFloat3(entities[i]->GetTransform()->GetPosition().get());
				DirectX::XMVECTOR vectorSub = DirectX::XMVectorSubtract(vector1, vector2);
				DirectX::XMVECTOR length = DirectX::XMVector3Length(vectorSub);

				DirectX::XMStoreFloat(&distance, length);
				// Don't add light if out of range 
				/*if (light->range < distance)
					continue;*/

				name = "pointLight" + std::to_string(pLights);
				pLights++;
				break;
			case LIGHT_TYPE_SPOT: // Not implemented yet 
			default:
				continue;
			}

			entities[i]->GetMat()->GetPixelShader()->SetData(
				name, // The name of the (eventual) variable in the shader
				lights[l].get(), // The address of the data to set
				sizeof(Light)); // The size of the data (the whole struct!) to set
		}

		entities[i]->Draw(context, cameras[currentCam]);
	}
}

void Scene::DrawSky(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	sky->Draw(cameras[currentCam]);
}

void Scene::DrawLightsGui(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	for (unsigned int i = 0; i < lightGizmos.size(); i++)
	{
		lightGizmos[i]->Draw(context, cameras[currentCam]);
	}
}

void Scene::ChangeCurrentCam(int index)
{
	// Safety 
	if (index >= 0 && index < cameras.size())
	{
		currentCam = index;
	}
}

void Scene::SetCameras(std::vector<std::shared_ptr<Camera>> cameras)
{
	(*this).cameras = cameras;
}

void Scene::SetEntities(std::vector<std::shared_ptr<Entity>> entities)
{
	(*this).entities = entities;
}

void Scene::SetSky(std::shared_ptr<Sky> sky)
{
	(*this).sky = sky;
}

void Scene::SetLights(std::vector<std::shared_ptr<Light>> lights)
{
	(*this).lights = lights;

	// Check for shadow type 
	for (int i = 0; i < lights.size(); i++)
	{
		if (!lights[i]->hasShadows)
			continue;

		// Make sure exists in the unordered list 
		if (lightToShadowData.find(lights[i].get()) == lightToShadowData.end())
			lightToShadowData[lights[i].get()] = std::make_shared<ShadadowShaderData>();
		
		// Update view matrix 
		
		DirectX::XMStoreFloat4x4(
			&lightToShadowData[lights[i].get()].get()->view, // Storing 
			XMMatrixLookToLH(
				-DirectX::XMLoadFloat3(&lights[i].get()->directiton) * 5, // Position: "Backing up" 20 units from origin
				DirectX::XMLoadFloat3(&lights[i].get()->directiton), // Direction: light's direction
				XMVectorSet(0, 1, 0, 0))); // Up: World up vector (Y axis)

		// Set projection
		DirectX::XMStoreFloat4x4(
			&lightToShadowData[lights[i].get()].get()->projection, // Storing 
			XMMatrixOrthographicLH(
				LIGHT_PROJ_SIZE,
				LIGHT_PROJ_SIZE,
				1.0f,
				100.0f));

	}
}

void Scene::SetLightsAndGui(std::vector<std::tuple<std::shared_ptr<Light>, std::shared_ptr<Entity>>> lightAndGui)
{
	for (int i = 0; i < lightAndGui.size(); i++)
	{
		std::tuple<std::shared_ptr<Light>, std::shared_ptr<Entity>> current = lightAndGui[i];
		lights.push_back(std::get<0>(current));
		lightGizmos.push_back(std::get<1>(current));

		lightToGizmos[lights[lights.size() - 1].get()] = lightGizmos[lightGizmos.size() - 1].get();
	}
}

void Scene::GenerateLightGizmos(
	std::shared_ptr<Mesh> lightMesh, 
	std::shared_ptr<SimpleVertexShader> vertex,
	std::shared_ptr<SimplePixelShader> pixel
	)
{
	// Create gizmos to represent lights in 3D space 
	for (int i = 0; i < lights.size(); i++)
	{
		Light* light = lights[i].get(); //i < sCount ? &spotLights[i] : &directionalLights[i - sCount];
		DirectX::XMFLOAT4 startColor = DirectX::XMFLOAT4(light->color.x, light->color.y, light->color.z, 1);

		// Light gizmos mat
		std::shared_ptr<Material> mat = std::make_shared<Material>(startColor, 1.0f, DirectX::XMFLOAT2(0, 0), vertex, pixel);

		// Add light gizmos to their own vector 
		lightGizmos.push_back(std::shared_ptr<Entity>(new Entity(lightMesh, mat)));
		lightGizmos[i]->GetTransform()->SetPosition(light->position);
		lightToGizmos[light] = lightGizmos[i].get();
	}
}

std::vector<std::shared_ptr<Entity>> Scene::GetEntities()
{
	return entities;
}

std::vector<std::shared_ptr<Light>> Scene::GetLights()
{
	return lights;
}

std::unordered_map<Light*, Entity*> Scene::GetLightToGizmos()
{
	return lightToGizmos;
}

std::vector<std::shared_ptr<Camera>> Scene::GetAllCams()
{
	return cameras;
}

std::shared_ptr<Camera> Scene::GetCurrentCam()
{
	return cameras[currentCam];
}

std::shared_ptr<ShadadowShaderData> Scene::GetShadowData(Light* light)
{
	if (lightToShadowData.find(light) == lightToShadowData.end())
		return nullptr;

	return lightToShadowData[light];
}


void Scene::ResizeCam(float windowWidth, float windowHeight)
{
	GetCurrentCam().get()->UpdateProjMatrix(
		DirectX::XM_PIDIV4,								// FOV 
		windowWidth / windowHeight				// Aspect Ratio
	);
}

std::string Scene::GetTitle()
{
	return sceneTitle;
}
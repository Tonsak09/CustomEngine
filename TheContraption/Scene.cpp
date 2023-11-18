#include "Scenes.h"

Scene::Scene(
	std::vector<std::shared_ptr<Camera>> cameras,
	std::vector<std::shared_ptr<Entity>> entities,
	std::vector<std::tuple<std::shared_ptr<Light>,std::shared_ptr<Entity>>> lightAndGui,
	std::shared_ptr<Sky> sky
) :
	cameras(cameras), entities(entities), lights(lights)
{
	// Start of cameras vector 
	currentCam = 0;

	// Split lights and their gui into two different vectors 
	SetLightsAndGui(lightAndGui);
}

Scene::Scene()
{
	currentCam = 0;

	lightToGizmos = std::unordered_map<Light*, Entity*>();
}


void Scene::DrawEntities(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	for (unsigned int i = 0; i < entities.size(); i++)
	{
		DirectX::XMFLOAT3 ambient(0.1f, 0.1f, 0.25f);
		entities[i]->GetMat()->GetPixelShader()->SetFloat3("ambient", ambient);

		int dLights = 1;
		int sLights = 1;
		int pLights = 1;
		for (int l = 0; l < lights.size(); l++)
		{
			int lightType = lights[l]->type;
			std::string name; //= (lights[l].type == 0 ? "directionalLight" : "spotLight") + std::to_string(l + 1);

			switch (lightType)
			{
			case LIGHT_TYPE_DIRECTIONAL:
				name = "directionalLight" + std::to_string(dLights);
				dLights++;
				break;
			case  LIGHT_TYPE_POINT:
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


void Scene::ResizeCam(float windowWidth, float windowHeight)
{
	GetCurrentCam().get()->UpdateProjMatrix(
		DirectX::XM_PIDIV4,								// FOV 
		windowWidth / windowHeight				// Aspect Ratio
	);
}
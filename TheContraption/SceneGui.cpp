#include "SceneGui.h"
using namespace DirectX;

SceneGui::SceneGui(){
	keyToEquation = std::unordered_map<std::string, int>();
}

void SceneGui::InstructionsGUI()
{
	ImGui::Begin("Instructions");
	ImGui::Text("This is an example of a render engine.");
	ImGui::Text("In the animation scene you have control over a simple translation animation that opens and closes a sphere with some componenents insides");
	ImGui::Text("The generic scene is used to for examples of the rendering ability of this engine and general testing");
	ImGui::End();
}

void SceneGui::CreateEntityGui(std::shared_ptr<Entity> entity)
{
	std::shared_ptr <Transform> trans = entity->GetTransform();
	XMFLOAT3 pos = *(trans->GetPosition().get());
	XMFLOAT3 rot = trans->GetEulerRotation();
	XMFLOAT3 sca = trans->GetScale();

	XMFLOAT2 uvOff = entity.get()->GetMat()->GetUVOffset();

	if (ImGui::DragFloat3("Position", &pos.x, 0.01f)) trans->SetPosition(pos);
	if (ImGui::DragFloat3("Rotation (Radians)", &rot.x, 0.01f)) trans->SetEulerRotation(rot);
	if (ImGui::DragFloat3("Scale", &sca.x, 0.01f)) trans->SetScale(sca);

	ImGui::Dummy(ImVec2(0, 10));
	if (ImGui::DragFloat2("UV Offset", &uvOff.x, 0.01f)) entity->GetMat()->SetUVOffset(uvOff);
}

void SceneGui::CreateLightGui(Light* light, Entity* lightGui)
{
	// GUI that all light types have 
	XMFLOAT3 color = light->color;
	XMFLOAT3 position = light->position;

	//ImGui::ColorEdit4(:Color)
	if (ImGui::ColorEdit3("Color", &color.x, 0.01f))
	{
		light->color = color;
		lightGui->GetMat()->SetTint(DirectX::XMFLOAT4(color.x, color.y, color.z, 1.0));
	}
	if (ImGui::DragFloat3("GUI Position", &position.x, 0.01f))
	{
		light->position = position;
		lightGui->GetTransform()->SetPosition(position);
	}

	// Create GUI information for specific light type 
	switch (light->type)
	{
	case LIGHT_TYPE_DIRECTIONAL:
		CreateDirLightGui(light);
		break;
	case LIGHT_TYPE_POINT:
		CreatePointLightGui(light);
		break;
	case LIGHT_TYPE_SPOT: // Currently no spot light 
	default:
		break;
	}
}

void SceneGui::CreateDirLightGui(Light* light)
{
	XMFLOAT3 direction = light->directiton;
	if (ImGui::DragFloat3("Direction", &direction.x, 0.01f)) light->directiton = direction;
}

void SceneGui::CreatePointLightGui(Light* light)
{
	float range = light->range;
	if (ImGui::DragFloat("Range", &range, 0.01f)) light->range = range;
}

void SceneGui::CreateSceneGui(std::vector<std::shared_ptr<Scene>> scenes, int* currentScene)
{
	// Display each scene 
	if (ImGui::TreeNode("Scenes"))
	{
		for (unsigned int i = 0; i < scenes.size(); i++)
		{
			ImGui::PushID(i);
			if (ImGui::Button("Scene", ImVec2(90, 25))) *currentScene = i;
			ImGui::PopID();
		}

		ImGui::TreePop();
	}
}

void SceneGui::UpdateLightGUI(std::vector<std::shared_ptr<Light>> lights, std::unordered_map<Light*, Entity*> lightToGizmos)
{
	// Display Light GUI

	for (unsigned int i = 0; i < lights.size(); i++)
	{
		ImGui::PushID(i);
		if (ImGui::TreeNode(lights[i]->type == LIGHT_TYPE_DIRECTIONAL ? "Directional" : "Point")) // TODO - Account for more light types 
		{
			CreateLightGui(lights[i].get(), lightToGizmos[lights[i].get()]);
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
}


void SceneGui::CreateCamGui(Camera* cam)
{
	float commonMoveSpeed = cam->GetCommonMoveSpeed();
	if (ImGui::DragFloat("Common Move Speed", &commonMoveSpeed, 0.01f)) cam->SetCommonMoveSpeed(commonMoveSpeed);
}

void SceneGui::UpdateEntityGUI(std::vector<std::shared_ptr<Entity>> entities)
{
	// Display Entity data 
	for (unsigned int i = 0; i < entities.size(); i++)
	{
		// Unique id
		ImGui::PushID(i);
		if (ImGui::TreeNode("Entity")) // How to make name based on id? 
		{
			CreateEntityGui(entities[i]);
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
}


void SceneGui::UpdateCameraGUI(std::vector<std::shared_ptr<Camera>> cameras, Scene *scene, float screenWidth, float screenHeight)
{
	const char* items[] = { "Cam0", "Cam1", "Cam2", "Cam3" };
	static const char* current_item = items[0];
	static int current = 0;

	if (ImGui::BeginCombo("Cameras", current_item)) // The second parameter is the label previewed before opening the combo.
	{
		for (int n = 0; n < IM_ARRAYSIZE(items); n++)
		{
			bool is_selected = (current_item != items[n]); // New item 
			if (ImGui::Selectable(items[n], is_selected))
			{
				current_item = items[n];
				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
					current = n;

					// Resize the screen 
					scene->ResizeCam(screenWidth, screenHeight);
				}
			}
		}
		ImGui::EndCombo();
	}
	CreateCamGui(cameras[current].get());
}

void SceneGui::CreateCurveGui(int curveType, float plotSizeX, float plotSizeY)
{
	ImVec2 size(plotSizeX, plotSizeY);

	float lines[120];


	// Actually displays the curve 
	for (int n = 0; n < 120; n++)
	{
		float p = n / 120.0f;

		lines[n] = GetCurveByIndex(curveType, p);
	}
	ImGui::PlotLines("AnimCurve", lines, 120, 0, (const char*)0, -0.24f, 1.25f, size);
}

void SceneGui::CreateCurveGuiWithDropDown(const char* key, int *value, float plotSizeX, float plotSizeY)
{
	static const char* items[] = {
		"EaseInSine", "EaseOutSine", "EaseInOutSine",
		"EaseInQuad", "EaseOutQuad", "EaseInOutQuad",
		"EaseInCubic", "EaseOutCubic", "EaseInOutCubic",
		"EaseInQuart", "EaseOutQuart", "EaseInOutQuart",
		"EaseInQuint", "EaseOutQuint", "EaseInOutQuint",
		"EaseInExpo", "EaseOutExpo", "EaseInOutExpo",
		"EaseInCirc", "EaseOutCirc", "EaseInOutCirc",
		"EaseInBack", "EaseOutBack", "EaseInOutBack",
		"EaseInElastic", "EaseOutElastic", "EaseInOutElastic",
		"EaseInBounce", "EaseOutBounce", "EaseInOutBounce",
	};

	if (keyToEquation.find(key) == keyToEquation.end())
	{
		keyToEquation[key] = *value;
	}

	//const char* current_item = items[0];
	//static int activeEquation = 0;
	ImGui::PushItemWidth(100.0f);
	// What type to display 
	if (ImGui::BeginCombo("Equation", items[keyToEquation[key]])) // The second parameter is the label previewed before opening the combo.
	{
		for (int n = 0; n < IM_ARRAYSIZE(items); n++)
		{
			bool is_selected = (items[keyToEquation[key]] != items[n]); // New item 
			if (ImGui::Selectable(items[n], is_selected))
			{
				keyToEquation[key] = n;
				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
					//activeEquation = n;

					break;
				}
			}
		}
		ImGui::EndCombo();
	}
	
	CreateCurveGui(keyToEquation[key]);

	*value = keyToEquation[key];
	//return keyToEquation[key];
}


#pragma once
#include <memory>

#include "Lights.h"
#include "Entity.h"
#include <DirectXMath.h>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

#include "Scenes.h"
#include "AnimCurves.h"

class SceneGui 
{
public:
	SceneGui();
	void UpdateEntityGUI(std::vector<std::shared_ptr<Entity>> entities);
	void UpdateLightGUI(std::vector<std::shared_ptr<Light>> lights, std::unordered_map<Light*, Entity*> lightToGizmos);
	void UpdateCameraGUI(std::vector<std::shared_ptr<Camera>> cameras, Scene* scene, float screenWidth, float screenHeight);

	void CreateEntityGui(std::shared_ptr<Entity> entity);
	/// <summary>
	/// Call this function to automatically create a light
	/// based on the lights type index
	/// </summary>
	/// <param name="light"></param>
	void CreateLightGui(Light* light, Entity* lightGui);
	void CreateDirLightGui(Light* light);
	void CreatePointLightGui(Light* light);

	/// <summary>
	/// Interact with adjustable settings for given camera
	/// </summary>
	/// <param name="cam"></param>
	void CreateCamGui(Camera* cam);

	/// <summary>
	/// Creates a visual curve for ImGui
	/// </summary>
	void CreateCurveGui(int curveType, float plotSizeX = 100.0f, float plotSizeY = 80.0f);
	/// <summary>
	/// Create a curve gui that automatically creates the dropdown and will return the selected index 
	/// </summary>
	/// <returns>Selected index. See AnimCurves.h for defines</returns>
	int CreateCurveGuiWithDropDown(float plotSizeX = 100.0f, float plotSizeY = 80.0f);

	private:

};
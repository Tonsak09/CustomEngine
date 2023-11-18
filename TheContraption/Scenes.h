#pragma once
#include "Entity.h"
#include "Lights.h"
#include "Sky.h"
#include <tuple>

#include "SimpleShader.h"
#include <DirectXMath.h>

/*
	The purpose of the script is to hold individual scene data that 
	lets us organize our game objects and to draw the appropriate 
	gui data for said scene 
*/

struct Scene
{
public:
	
	Scene(
		std::vector<std::shared_ptr<Camera>> cameras,
		std::vector<std::shared_ptr<Entity>> entities,
		// We pass in light and its gui at the same time so
		// that they have the same size.
		std::vector<std::tuple<std::shared_ptr<Light>, std::shared_ptr<Entity>>> lightAndGui,
		std::shared_ptr<Sky> sky
	);

	Scene();

	void DrawEntities(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);
	void DrawSky(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);
	void DrawLightsGui(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);
	void DrawImGui();

	void ChangeCurrentCam(int index);
	void SetCameras(std::vector<std::shared_ptr<Camera>> cameras);
	void SetEntities(std::vector<std::shared_ptr<Entity>> entities);
	void SetLightsAndGui(std::vector<std::tuple<std::shared_ptr<Light>, std::shared_ptr<Entity>>> lightAndGui);
	void SetLights(std::vector<std::shared_ptr<Light>> lights);
	void SetSky(std::shared_ptr<Sky> sky);

	void ResizeCam(float windowWidth, float windowHeight);

	// Recreate the gizmos for light objects 
	// using the given mesh
	void GenerateLightGizmos(
		std::shared_ptr<Mesh> lightMesh, 
		std::shared_ptr<SimpleVertexShader> vertex,
		std::shared_ptr<SimplePixelShader> pixel
	);

	std::vector<std::shared_ptr<Entity>> GetEntities();
	std::vector<std::shared_ptr<Light>> GetLights();
	std::unordered_map<Light*, Entity*> GetLightToGizmos();
	std::vector<std::shared_ptr<Camera>> GetAllCams();
	std::shared_ptr<Camera> GetCurrentCam();

private:
	// World entities 
	std::vector<std::shared_ptr<Entity>> entities;

	std::shared_ptr<Sky> sky;

	// Camera 
	int currentCam;
	std::vector<std::shared_ptr<Camera>> cameras;

	// Display light positions 
	std::vector<std::shared_ptr<Light>> lights;
	std::vector<std::shared_ptr<Entity>> lightGizmos;
	std::unordered_map<Light*, Entity*> lightToGizmos; 
};
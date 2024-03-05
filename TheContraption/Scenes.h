#pragma once
#include "Entity.h"
#include "Lights.h"
#include "Sky.h"
#include <tuple>

#include "SimpleShader.h"
#include <DirectXMath.h>
#include <string>

#include "ShadowShaderData.h"

/*
	The purpose of the script is to hold individual scene data that 
	lets us organize our game objects and to draw the appropriate 
	gui data for said scene 
*/

#define LIGHT_PROJ_SIZE 15.0f

struct Scene
{
public:
	
	Scene(
		std::string sceneTitle,
		std::vector<std::shared_ptr<Camera>> cameras,
		std::vector<std::shared_ptr<Entity>> entities,
		// We pass in light and its gui at the same time so
		// that they have the same size.
		std::vector<std::tuple<std::shared_ptr<Light>, std::shared_ptr<Entity>>> lightAndGui,
		std::shared_ptr<Sky> sky
	);

	Scene(std::string sceneTitle);

	void DrawShadows(
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufferRTV,
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthBufferDSV,
		std::shared_ptr<SimpleVertexShader> shadowVS,
		int shadowResolutionSize,
		float windowWidth, float windowHeight);
	void DrawEntities(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);
	void DrawDebugEntities(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);
	void DrawSky(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);
	void DrawLightsGui(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);
	void DrawImGui();

	void ChangeCurrentCam(int index);
	void SetCameras(std::vector<std::shared_ptr<Camera>> cameras);
	void SetEntities(std::vector<std::shared_ptr<Entity>> entities);
	void SetDebugEntities(std::vector<std::shared_ptr<Entity>> entities);
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
	std::vector<std::shared_ptr<Entity>> GetDebugEntities();
	std::vector<std::shared_ptr<Light>> GetLights();
	std::unordered_map<Light*, Entity*> GetLightToGizmos();
	std::vector<std::shared_ptr<Camera>> GetAllCams();
	std::shared_ptr<Camera> GetCurrentCam();
	std::shared_ptr<ShadadowShaderData> GetShadowData(Light* light);

	std::string GetTitle();

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV;

private:
	std::string sceneTitle;

	// World entities 
	std::vector<std::shared_ptr<Entity>> entities;
	std::vector<std::shared_ptr<Entity>> debugEntities;

	std::shared_ptr<Sky> sky;

	// Camera 
	int currentCam;
	std::vector<std::shared_ptr<Camera>> cameras;

	// Display light positions 
	std::vector<std::shared_ptr<Light>> lights;
	std::vector<std::shared_ptr<Entity>> lightGizmos;
	std::unordered_map<Light*, Entity*> lightToGizmos; 

	// Holds data for creating shadow map on lights
	// that want to have shadows 
	std::unordered_map<Light*, std::shared_ptr<ShadadowShaderData>> lightToShadowData;

};
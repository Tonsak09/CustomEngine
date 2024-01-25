#pragma once

#define SHOW_GUI_ENTITIES 0
#define SHOW_GUI_LIGHTS 1
#define SHOW_GUI_CAMERA 2

// Scenes 
#define SCENE_PRIMARY 0
#define SCENE_ANIM 1
#define SCENE_SHADOWS 2
#define SCENE_SKELE 3

#define SHADOW_MAP_RESOLUTION 1024

#define MAX_DITHERS 5

#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects

#include <memory>
#include <thread>
#include <algorithm>

#include <vector>
#include "Entity.h"
#include "Camera.h"

#include "SimpleShader.h"
#include "Material.h"
#include "packages/directxtk_desktop_win10.2023.9.6.1/include/WICTextureLoader.h"

#include "Lights.h"
#include "MatData.h"

#include "Sky.h"

#include "AnimCurves.h"
#include "Scenes.h"
#include "SceneGui.h"

#include "BasicAnimation.h"

class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

private:
	void LoadLights();
	void LoadShaders(); 
	void CreateGeometry();
	void CreateCameras();
	void LoadShadowResources();

	/// <summary>
	/// Helper for creating light view matricies for a light
	/// </summary>
	DirectX::XMMATRIX CreateLightViewMatrix(Light light);


	// Logic specifically for animation demonstration
	void AnimSceneLogic(float deltaTime);

	// Gui - Used to tell the computer which gui to display 
	void UpdateImGui(float deltaTime);

	void SetupLitMaterial(
		std::shared_ptr<Material> mat, 
		const wchar_t albedoTextureAddress[],
		const wchar_t speculuarMapAddress[],
		const wchar_t normalMapAddress[],
		const char samplerType[] = "BasicSampler"
		);

	void SetupPBRMaterial(
		std::shared_ptr<Material> mat,
		const wchar_t albedoTextureAddress[],
		const wchar_t normalMapAddress[],
		const wchar_t roughnessMapAddress[],
		const wchar_t metalMapAddress[],
		Sky* sky,
		const char samplerType[] = "BasicSampler"
	);

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rustyMetal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rustyMetalSpec;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> blankNormal;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> rustyMetalSamplerState;


	// Shaders and shader-related constructs
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimplePixelShader> litShader;
	std::shared_ptr<SimplePixelShader> schlickShader;
	std::shared_ptr<SimplePixelShader> customPShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimpleVertexShader> shadowVS;

	// Materials 
	std::shared_ptr<Material> mat1;
	std::shared_ptr<Material> mat2;
	std::shared_ptr<Material> mat3;
	std::shared_ptr<Material> lit;

	// PBR's
	std::shared_ptr<Material> schlickBricks;
	std::shared_ptr<Material> rough;
	std::shared_ptr<Material> schlickBronze;
	std::shared_ptr<Material> wood;
	std::vector<std::shared_ptr<Material>> MaterialsPBR;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSampler;

	std::unordered_map<std::shared_ptr<Material>, std::shared_ptr<MatData>> matToResources;

	// Light gizmo itmes 
	std::vector<std::shared_ptr<Entity>> lightGizmos;
	std::unordered_map<Light*, Entity*> lightToGizmos;
	
	// Changes between entities, lights, and camera 
	int currentGUI;

	// Lights 
	Light directionalLight1;
	Light directionalLight2;
	Light directionalLight3;
	Light pointLight1;
	Light pointLight2;

	#pragma region SHADOWS
	
	//DirectX::XMFLOAT4X4 shadowViewMatrix;
	//DirectX::XMFLOAT4X4 shadowProjectionMatrix;
	#pragma endregion
	

	/*
		New scenes require both a scene and a sceneGui when
		working within debug mode along with scene constructor
		requirements.

		Since not all items can be given into the constructor
		easily we use setter functions to initialize our scene
		piece by piece BEFORE we start drawing it 
	*/

	// Scene Mangement 
	int currentScene;
	std::vector<std::shared_ptr<Scene>> scenes;
	std::vector<std::shared_ptr<SceneGui>> sceneGuis;

	// Primary Scene 
	std::shared_ptr<Scene> scene;
	std::shared_ptr<SceneGui> sceneGui; // Debug info 
	
	#pragma region ANIMATION_SCENE

	// Animation Testing Scene 
	std::shared_ptr<Scene> animScene;
	std::shared_ptr<SceneGui> animSceneGui;

	std::shared_ptr<BasicAnimationManager> animManager;
	bool startAnimation; // Whether to start animation or not 
	float buttonCooldown;

	// Seperate 
	float eyeSepTime;
	int eyeSepCurve;

	// Combine 
	float eyeComTime;
	int eyeComCurve;
	#pragma endregion

	// Shadow Scene 
	std::shared_ptr<Scene> shadowScene;
	std::shared_ptr<SceneGui> shadowSceneGui;

	// Skeletal Animation Scene 
	std::shared_ptr<Scene> skeleScene;
	std::shared_ptr<SceneGui> skeleSceneGui;

	// Dithering 
	float ditherAmount; 
};
#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"

#include <memory>
#include "Mesh.h"
#include "Transform.h"


// Assumes files are in "ImGui" subfolder!
// Adjust path as necessary
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// Direct3D itself, and our window, are not ready at this point!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,			// The application's handle
		L"DirectX Game",	// Text for the window's title bar (as a wide-character string)
		720,				// Width of the window's client area
		720,				// Height of the window's client area
		false,				// Sync the framerate to the monitor refresh? (lock framerate)
		true)				// Show extra stats (fps) in title bar?
{
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");

	scene = std::make_shared<Scene>("General");
	sceneGui = std::make_shared<SceneGui>(scene);

	animScene = std::make_shared<Scene>("Anim");
	animSceneGui = std::make_shared<SceneGui>(animScene);
	animManager = std::make_shared<BasicAnimationManager>();

	shadowScene = std::make_shared<Scene>("Shadow");
	shadowSceneGui = std::make_shared<SceneGui>(shadowScene);

	scenes.push_back(scene);
	scenes.push_back(animScene);
	scenes.push_back(shadowScene);
	sceneGuis.push_back(sceneGui);
	sceneGuis.push_back(animSceneGui);
	sceneGuis.push_back(shadowSceneGui);


	currentGUI = 0; currentScene = SCENE_SHADOWS;
	
	// Defaults 
	eyeSepTime = 1.5f;
	eyeSepCurve = EASE_OUT_ELASTIC;
	eyeComTime = 1.0f;
	eyeComCurve = EASE_IN_BOUNCE;

	buttonCooldown = 2.0f;

#endif
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Delete all objects manually created within this class
//  - Release() all Direct3D objects created within this class
// --------------------------------------------------------
Game::~Game()
{
	// Call delete or delete[] on any objects or arrays you've
	// created using new or new[] within this class
	// - Note: this is unnecessary if using smart pointers

	// Call Release() on any Direct3D objects made within this class
	// - Note: this is unnecessary for D3D objects stored in ComPtrs
	
	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

}

// --------------------------------------------------------
// Called once per program, after Direct3D and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	LoadLights();
	LoadShaders();
	LoadShadowResources();
	CreateGeometry();
	CreateCameras();
	
	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

// Ensure the pipeline knows how to interpret all the numbers stored in
// the vertex buffer. For this course, all of your vertices will probably
// have the same layout, so we can just set this once at startup.
//context->IASetInputLayout(inputLayout.Get());
	}

	// Initialize ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	// Pick a style (uncomment one of these 3)
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();
	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(device.Get(), context.Get());


	OnResize();
}

/// <summary>
/// Create the different lights for the scene 
/// </summary>
void Game::LoadLights()
{
	#pragma region scene1
	std::vector<std::shared_ptr<Light>> lights = std::vector<std::shared_ptr<Light>>();

		directionalLight1 = {};
		directionalLight1.type = LIGHT_TYPE_DIRECTIONAL;
		directionalLight1.directiton = DirectX::XMFLOAT3(1, -1, 0);
		directionalLight1.color = DirectX::XMFLOAT3(0.2f, 0.2f, 0.2f);
		directionalLight1.intensity = 1.0;
		directionalLight1.hasShadows = true;
		lights.push_back(std::make_shared<Light>(directionalLight1));

		directionalLight2 = {};
		directionalLight2.type = LIGHT_TYPE_DIRECTIONAL;
		directionalLight2.directiton = DirectX::XMFLOAT3(0, -1, 0);
		directionalLight2.color = DirectX::XMFLOAT3(1, 1, 1);
		directionalLight2.intensity = 1.0;
		directionalLight1.hasShadows = false;
		lights.push_back(std::make_shared<Light>(directionalLight2));

		directionalLight3 = {};
		directionalLight3.type = LIGHT_TYPE_DIRECTIONAL;
		directionalLight3.directiton = DirectX::XMFLOAT3(-0.2f, 1, 0);
		directionalLight3.color = DirectX::XMFLOAT3(0, 1, 0);
		directionalLight3.intensity = 1.0;
		directionalLight1.hasShadows = false;
		lights.push_back(std::make_shared<Light>(directionalLight3));

		pointLight1 = {};
		pointLight1.type = LIGHT_TYPE_POINT;
		pointLight1.position = DirectX::XMFLOAT3(2, 2, 0);
		pointLight1.color = DirectX::XMFLOAT3(0, 1, 0);
		pointLight1.intensity = 1.0;
		pointLight1.range = 40.0;
		directionalLight1.hasShadows = false;
		lights.push_back(std::make_shared<Light>(pointLight1));

		pointLight2 = {};
		pointLight2.type = LIGHT_TYPE_POINT;
		pointLight2.position = DirectX::XMFLOAT3(1, -3, 0);
		pointLight2.color = DirectX::XMFLOAT3(0, 0, 1);
		pointLight2.intensity = 1.0;
		pointLight2.range = 100.0;
		directionalLight1.hasShadows = false;
		lights.push_back(std::make_shared<Light>(pointLight2));

		// Set the scene's lights 
		scene->SetLights(lights);
	#pragma endregion


	#pragma region AnimScene
	std::vector<std::shared_ptr<Light>> lights2 = std::vector<std::shared_ptr<Light>>();

	directionalLight1 = {};
	directionalLight1.type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight1.directiton = DirectX::XMFLOAT3(1, -1, 0);
	directionalLight1.color = DirectX::XMFLOAT3(0.2f, 0.2f, 0.2f);
	directionalLight1.intensity = 1.0;
	directionalLight1.hasShadows = true;
	lights2.push_back(std::make_shared<Light>(directionalLight1));

	pointLight1 = {};
	pointLight1.type = LIGHT_TYPE_POINT;
	pointLight1.position = DirectX::XMFLOAT3(2, 2, 0);
	pointLight1.color = DirectX::XMFLOAT3(0, 1, 0);
	pointLight1.intensity = 1.0;
	pointLight1.range = 40.0;
	directionalLight1.hasShadows = false;
	lights2.push_back(std::make_shared<Light>(pointLight1));

	// Set the scene's lights 
	animScene->SetLights(lights2);
	#pragma endregion

	#pragma region ShadowScene
	std::vector<std::shared_ptr<Light>> lightsShadow = std::vector<std::shared_ptr<Light>>();

	Light shadDir = {};
	shadDir.type = LIGHT_TYPE_DIRECTIONAL;
	shadDir.directiton = DirectX::XMFLOAT3(1, -1, -0.5f);
	shadDir.color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	shadDir.intensity = 1.0;
	shadDir.hasShadows = true;
	lightsShadow.push_back(std::make_shared<Light>(shadDir));

	Light shadPoint = {};
	shadPoint.type = LIGHT_TYPE_POINT;
	shadPoint.position = DirectX::XMFLOAT3(2, 2, 0);
	shadPoint.color = DirectX::XMFLOAT3(0, 1, 0);
	shadPoint.intensity = 0.5;
	shadPoint.range = 10.0;
	shadPoint.hasShadows = false;
	lightsShadow.push_back(std::make_shared<Light>(shadPoint));

	shadowScene->SetLights(lightsShadow);
	#pragma endregion
}

void Game::SetupLitMaterial(
	std::shared_ptr<Material> mat,
	const wchar_t albedoTextureAddress[],
	const wchar_t speculuarMapAddress[],
	const wchar_t normalMapAddress[],
	const char samplerType[])
{
	// Create the data storage struct 
	std::shared_ptr<MatData> tempData = std::make_shared<MatData>(device);
	matToResources[mat] = tempData;
	MatData* data = tempData.get();

	// Load in the textures and store them into matdata
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(albedoTextureAddress).c_str(), nullptr, data->albedo.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(speculuarMapAddress).c_str(), nullptr, data->spec.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(normalMapAddress).c_str(), nullptr, data->normal.GetAddressOf());

	// Apply to shader registers 
	mat.get()->AddSampler("BasicSampler", sampler);
	mat.get()->AddTextureSRV("SurfaceTexture", data->albedo);
	mat.get()->AddTextureSRV("NormalMap", data->normal);
	mat.get()->AddTextureSRV("SpeculuarTexture", data->spec);
}

void Game::SetupPBRMaterial(
	std::shared_ptr<Material> mat,
	const wchar_t albedoTextureAddress[],
	const wchar_t normalMapAddress[],
	const wchar_t roughnessMapAddress[],
	const wchar_t metalMapAddress[],
	Sky* sky,
	const char samplerType[])
{
	// Create the data storage struct 
	std::shared_ptr<MatData> tempData = std::make_shared<MatData>(device);
	matToResources[mat] = tempData;
	MatData* data = tempData.get();

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> dither;

	// Load in the textures and store them into matdata
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(albedoTextureAddress).c_str(), nullptr, data->albedo.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(normalMapAddress).c_str(), nullptr, data->normal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(roughnessMapAddress).c_str(), nullptr, data->roughness.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(metalMapAddress).c_str(), nullptr, data->metal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/Dither/BayerDither8x8.png").c_str(), nullptr, dither.GetAddressOf());

	// Apply to shader registers 
	mat.get()->AddSampler("BasicSampler", sampler);
	mat.get()->AddSampler("ShadowSampler", shadowSampler);
	mat.get()->AddTextureSRV("Albedo", data->albedo);
	mat.get()->AddTextureSRV("NormalMap", data->normal);
	mat.get()->AddTextureSRV("RoughnessMap", data->roughness);
	mat.get()->AddTextureSRV("MetalnessMap", data->metal);
	mat.get()->AddTextureSRV("Environment", sky->GetCubeSRV());
	mat.get()->AddTextureSRV("Dither", dither);
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	vertexShader = std::make_shared<SimpleVertexShader>(device, context,
		FixPath(L"VertexShader.cso").c_str());
	shadowVS = std::make_shared< SimpleVertexShader>(device, context,
		FixPath(L"ShadowMapVertexShader.cso").c_str());
	pixelShader = std::make_shared<SimplePixelShader>(device, context,
		FixPath(L"PixelShader.cso").c_str());
	customPShader = std::make_shared<SimplePixelShader>(device, context,
		FixPath(L"CustomPS.cso").c_str());
	litShader = std::make_shared<SimplePixelShader>(device, context,
		FixPath(L"litPS.cso").c_str());
	schlickShader = std::make_shared< SimplePixelShader>(device, context,
		FixPath(L"Schlick.cso").c_str());
	
}


// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateGeometry()
{
	D3D11_SAMPLER_DESC sampDesc = {};

	// How to handles uvs going outside of range 
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	// Other options: 
	// D3D11_FILTER_MIN_MAG_MIP_LINEAR 
	// D3D11_FILTER_MIN_MAG_MIP_POINT 
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC; 
	sampDesc.MaxAnisotropy = 16;

	// Range of mip maping where 0 is it turned off 
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Creates the sampler 
	device.Get()->CreateSamplerState(&sampDesc, sampler.GetAddressOf());
	
	#pragma region MODELS

	// General Models 
	std::shared_ptr<Mesh> sphere = std::make_shared<Mesh>(device, context, FixPath(L"../../Assets/Models/sphere.obj").c_str());
	std::shared_ptr<Mesh> helix = std::make_shared<Mesh>(device, context, FixPath(L"../../Assets/Models/helix.obj").c_str());
	std::shared_ptr<Mesh> cube = std::make_shared<Mesh>(device, context, FixPath(L"../../Assets/Models/cube.obj").c_str());
	std::shared_ptr<Mesh> torus = std::make_shared<Mesh>(device, context, FixPath(L"../../Assets/Models/torus.obj").c_str());
	std::shared_ptr<Mesh> quad = std::make_shared<Mesh>(device, context, FixPath(L"../../Assets/Models/quad_double_sided.obj").c_str());
	std::shared_ptr<Mesh> lightGUIModel = std::make_shared<Mesh>(device, context, FixPath(L"../../Assets/Models/LightGUIModel.obj").c_str());

	// Setup mec eye 
	std::shared_ptr<Mesh> connectionA = std::make_shared<Mesh>(device, context, FixPath(L"../../Assets/Models/MecEye/ConnectionA.obj").c_str());
	std::shared_ptr<Mesh> connectionB = std::make_shared<Mesh>(device, context, FixPath(L"../../Assets/Models/MecEye/ConnectionB.obj").c_str());
	std::shared_ptr<Mesh> connectionC = std::make_shared<Mesh>(device, context, FixPath(L"../../Assets/Models/MecEye/ConnectionC.obj").c_str());
	std::shared_ptr<Mesh> Eye_Front = std::make_shared<Mesh>(device, context, FixPath(L"../../Assets/Models/MecEye/Eye_Front.obj").c_str());
	std::shared_ptr<Mesh> Eye_Mid = std::make_shared<Mesh>(device, context, FixPath(L"../../Assets/Models/MecEye/Eye_Mid.obj").c_str());
	std::shared_ptr<Mesh> Eye_Back = std::make_shared<Mesh>(device, context, FixPath(L"../../Assets/Models/MecEye/Eye_Back.obj").c_str());
	std::shared_ptr<Mesh> HeatSink = std::make_shared<Mesh>(device, context, FixPath(L"../../Assets/Models/MecEye/HeatSink.obj").c_str());
	std::shared_ptr<Mesh> Microchip = std::make_shared<Mesh>(device, context, FixPath(L"../../Assets/Models/MecEye/Microchip.obj").c_str());

	#pragma endregion

	#pragma region SETUP_MATERIALS

	mat1 = std::make_shared<Material>(DirectX::XMFLOAT4(1, 1, 1, 1), 1.0f, DirectX::XMFLOAT2(0, 0), vertexShader, customPShader);
	mat2 = std::make_shared<Material>(DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1), 1.0f, DirectX::XMFLOAT2(0, 0), vertexShader, pixelShader);
	mat3 = std::make_shared<Material>(DirectX::XMFLOAT4(1, 1, 0, 1), 1.0f, DirectX::XMFLOAT2(0, 0), vertexShader, pixelShader);
	lit = std::make_shared<Material>(DirectX::XMFLOAT4(1, 1, 1, 1), 0.5f, DirectX::XMFLOAT2(0, 0), vertexShader, litShader);
	//litCushion = std::make_shared<Material>(DirectX::XMFLOAT4(1, 1, 1, 1), 0.5f, DirectX::XMFLOAT2(0, 0), vertexShader, litShader);

	schlickBricks = std::make_shared<Material>(DirectX::XMFLOAT4(1, 1, 1, 1), 0.5f, DirectX::XMFLOAT2(0, 0), vertexShader, schlickShader);
	schlickCushions = std::make_shared<Material>(DirectX::XMFLOAT4(1, 1, 1, 1), 0.5f, DirectX::XMFLOAT2(0, 0), vertexShader, schlickShader);
	schlickBronze = std::make_shared<Material>(DirectX::XMFLOAT4(1, 1, 1, 1), 0.5f, DirectX::XMFLOAT2(0, 0), vertexShader, schlickShader);

	std::shared_ptr<Sky> sky = std::make_shared<Sky>(
		device,
		context,
		sampler,
		cube,
		L"../../Assets/Textures/Skies/Planet/right.png",
		L"../../Assets/Textures/Skies/Planet/left.png",
		L"../../Assets/Textures/Skies/Planet/up.png",
		L"../../Assets/Textures/Skies/Planet/down.png",
		L"../../Assets/Textures/Skies/Planet/front.png",
		L"../../Assets/Textures/Skies/Planet/back.png"
		);

	scene->SetSky(sky);
	animScene->SetSky(sky);
	shadowScene->SetSky(sky);

	// Create materials 
	SetupLitMaterial(
		lit,
		L"../../Assets/Textures/rustymetal.png",
		L"../../Assets/Textures/rustymetal_specular.png",
		L"../../Assets/Textures/original.png"
	);

	SetupPBRMaterial(
		schlickBronze,
		L"../../Assets/Textures/PBR/bronze_albedo.png",
		L"../../Assets/Textures/PBR/bronze_normals.png",
		L"../../Assets/Textures/PBR/bronze_roughness",
		L"../../Assets/Textures/PBR/bronze_metal",
		sky.get()
	);

	SetupPBRMaterial(
		schlickBricks,
		L"../../Assets/Textures/PBR/cobblestone_albedo.png",	// Albedo
		L"../../Assets/Textures/PBR/cobblestone_normals.png",	// Normals
		L"../../Assets/Textures/PBR/cobblestone_roughness",		// Roughness
		L"../../Assets/Textures/PBR/cobblestone_metal",			// Metalness 
		sky.get()
	);

	SetupPBRMaterial(
		schlickCushions,
		L"../../Assets/Textures/PBR/wood_albedo.png",
		L"../../Assets/Textures/PBR/wood_normals.png",
		L"../../Assets/Textures/PBR/wood_roughness",
		L"../../Assets/Textures/PBR/wood_metal",
		sky.get()
	);
	
	#pragma endregion

	#pragma region GENERIC_SCENE_ENTITIES

	std::vector<std::shared_ptr<Entity>> entities = std::vector<std::shared_ptr<Entity>>();

	// Add all entites to the primary vector 
	entities.push_back(std::shared_ptr<Entity>(new Entity(helix, schlickBricks)));
	entities[0]->GetTransform()->SetPosition(1.0f, 0.0f, 0.0f);

	entities.push_back(std::shared_ptr<Entity>(new Entity(sphere, schlickBronze)));
	entities[1]->GetTransform()->MoveRelative(5.0f, 0.0f, 0.0f);

	entities.push_back(std::shared_ptr<Entity>(new Entity(cube, schlickCushions)));
	entities[2]->GetTransform()->MoveRelative(-5.0f, 0.0f, 0.0f);

	// Put all into scene(s)
	scene->SetEntities(entities);
	scene->GenerateLightGizmos(lightGUIModel, vertexShader, pixelShader);

	#pragma endregion

	#pragma region ANIM_SCENE_ENTITIES

	// Animation scene 
	std::vector<std::shared_ptr<Entity>> entities2 = std::vector<std::shared_ptr<Entity>>();
	//entities2.push_back(std::shared_ptr<Entity>(new Entity(sphere, schlickBronze)));


	entities2.push_back(std::shared_ptr<Entity>(new Entity(connectionA, schlickBronze)));
	entities2.push_back(std::shared_ptr<Entity>(new Entity(connectionB, schlickBronze)));
	entities2.push_back(std::shared_ptr<Entity>(new Entity(connectionC, schlickBronze)));
	entities2.push_back(std::shared_ptr<Entity>(new Entity(Eye_Front, schlickBronze)));
	entities2.push_back(std::shared_ptr<Entity>(new Entity(Eye_Mid, schlickBronze)));
	entities2.push_back(std::shared_ptr<Entity>(new Entity(Eye_Back, schlickBronze)));
	entities2.push_back(std::shared_ptr<Entity>(new Entity(HeatSink, schlickBronze)));
	entities2.push_back(std::shared_ptr<Entity>(new Entity(Microchip, schlickBronze)));


	animScene->SetEntities(entities2);
	animScene->GenerateLightGizmos(lightGUIModel, vertexShader, pixelShader);

	#pragma endregion


	#pragma region GENERIC_SCENE_ENTITIES

	std::vector<std::shared_ptr<Entity>> shadowEntities = std::vector<std::shared_ptr<Entity>>();

	// Add all entites to the primary vector 
	shadowEntities.push_back(std::shared_ptr<Entity>(new Entity(helix, schlickBricks)));
	shadowEntities[0]->GetTransform()->SetPosition(1.0f, 0.0f, 0.0f);

	shadowEntities.push_back(std::shared_ptr<Entity>(new Entity(sphere, schlickBronze)));
	shadowEntities[1]->GetTransform()->MoveRelative(5.0f, 0.0f, 0.0f);

	shadowEntities.push_back(std::shared_ptr<Entity>(new Entity(cube, schlickCushions)));
	shadowEntities[2]->GetTransform()->MoveRelative(-5.0f, 0.0f, 0.0f);
	
	shadowEntities.push_back(std::shared_ptr<Entity>(new Entity(quad, schlickCushions)));
	shadowEntities[3]->GetTransform()->MoveRelative(0.0f, -2.0f, 0.0f);
	shadowEntities[3]->GetTransform()->SetScale(DirectX::XMFLOAT3(10, 1, 10));

	// Put all into scene(s)
	shadowScene->SetEntities(shadowEntities);
	shadowScene->GenerateLightGizmos(lightGUIModel, vertexShader, pixelShader);

	#pragma endregion
}


void Game::CreateCameras()
{
	std::vector<std::shared_ptr<Camera>> cameras = std::vector<std::shared_ptr<Camera>>();

	// Create the cameras 
	cameras.push_back(std::make_shared<Camera>(
		0.0f, 0.0f, -5.0f,						// Pos
		1.0f,									// Move speed
		10.0f,									// Sprint speed (hold left shift)
		0.1f,									// Mouse look speed 
		XM_PIDIV4,								// FOV 
		this->windowWidth / this->windowHeight	// Aspect ratio 
		));

	cameras.push_back(std::make_shared<Camera>(
		0.0f, 0.0f, -20.0f,						// Pos
		3.0f,									// Move speed
		10.0f,									// Sprint speed (hold left shift)
		0.1f,									// Mouse look speed 
		XM_PIDIV4,								// FOV 
		this->windowWidth / this->windowHeight	// Aspect ratio 
		));

	cameras.push_back(std::make_shared<Camera>(
		2.0f, 1.5f, -15.0f,						// Pos
		1.0f,									// Move speed
		10.0f,									// Sprint speed (hold left shift)
		0.1f,									// Mouse look speed 
		XM_PI / 8,								// FOV 
		this->windowWidth / this->windowHeight	// Aspect ratio 
		));

	cameras.push_back(std::make_shared<Camera>(
		-1.0f, 1.0f, -5.0f,						// Pos
		1.0f,									// Move speed
		10.0f,									// Sprint speed (hold left shift)
		0.1f,									// Mouse look speed 
		XM_PIDIV2,								// FOV 
		this->windowWidth / this->windowHeight	// Aspect ratio 
		));

	scene->SetCameras(cameras);
	animScene->SetCameras(cameras);
	shadowScene->SetCameras(cameras);
}


void Game::LoadShadowResources()
{
	


	// Create the actual texture that will be the shadow map
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = SHADOW_MAP_RESOLUTION; // Ideally a power of 2 (like 1024)
	shadowDesc.Height = SHADOW_MAP_RESOLUTION; // Ideally a power of 2 (like 1024)
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	device->CreateTexture2D(&shadowDesc, 0, shadowTexture.GetAddressOf());


	// Create the depth/stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D; // Says dimsions is 2D
	shadowDSDesc.Texture2D.MipSlice = 0;						// Which mipmap to take, in our case first one
	device->CreateDepthStencilView(
		shadowTexture.Get(),
		&shadowDSDesc,
		scenes[currentScene]->shadowDSV.GetAddressOf());

	// Create the SRV for the shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(
		shadowTexture.Get(),
		&srvDesc,
		scenes[currentScene]->shadowSRV.GetAddressOf());


	// Shadow sampler 
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f; // Only need the first component
	device->CreateSamplerState(&shadowSampDesc, &shadowSampler);
}

void Game::AnimSceneLogic(float deltaTime)
{
	//deltaTime /= 100.0f;

	// Whether the object has been broken apart 
	static bool isSplit = false;

	static DirectX::XMFLOAT3 rest	(0, 0,	0	);
	static DirectX::XMFLOAT3 front	(0, 0,	1.0f);
	static DirectX::XMFLOAT3 back	(0, 0, -1.0f);

	if (animManager->IsRunningAnimations())
	{
		startAnimation = false;
		return;
	}


	if (startAnimation) // Whether to begin the animation 
	{
		// Setup for animation 
		startAnimation = false;

		// Following sets up start and end animations for eye 
		animManager->AddAnimation( // Eye_Front
			animScene->GetEntities()[3]->GetTransform(),
			isSplit ? front : rest,
			isSplit ? rest : front,
			isSplit ? eyeComTime : eyeSepTime,
			isSplit ? eyeComCurve : eyeSepCurve
		);

		animManager->AddAnimation( // Eye_Back
			animScene->GetEntities()[5]->GetTransform(),
			isSplit ? back : rest,
			isSplit ? rest : back,
			isSplit ? eyeComTime : eyeSepTime,
			isSplit ? eyeComCurve : eyeSepCurve
		);

		animManager->AddAnimation( // Heatsink
			animScene->GetEntities()[6]->GetTransform(),
			isSplit ? back : rest,
			isSplit ? rest : back,
			isSplit ? eyeComTime : eyeSepTime,
			isSplit ? eyeComCurve : eyeSepCurve
		);

		animManager->AddAnimation( // Microchip
			animScene->GetEntities()[7]->GetTransform(),
			isSplit ? back : rest,
			isSplit ? rest : back,
			isSplit ? eyeComTime : eyeSepTime,
			isSplit ? eyeComCurve : eyeSepCurve
		);


		isSplit = !isSplit;
	}
	
}

// --------------------------------------------------------
// Handle resizing to match the new window size.
//  - DXCore needs to resize the back buffer
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();
	 
	scene->ResizeCam((float)this->windowWidth, (float)this->windowHeight);
}

void Game::UpdateImGui(float deltaTime)
{
	// Feed fresh input data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)this->windowWidth;
	io.DisplaySize.y = (float)this->windowHeight;


	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Determine new input capture
	Input& input = Input::GetInstance();
	input.SetKeyboardCapture(io.WantCaptureKeyboard);
	input.SetMouseCapture(io.WantCaptureMouse);


	float frameRate = ImGui::GetIO().Framerate;
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
		1000.0 / frameRate, frameRate);
	ImGui::Text("Window Width: %i", windowWidth);
	ImGui::Text("Window Height: %i", windowHeight);

	// Scene Management
	sceneGui->CreateSceneGui(scenes, &currentScene);
	sceneGui->InstructionsGUI();

	// Each scene has these settings
	if (ImGui::TreeNode("Scene Objects"))
	{
		
		// What Scene specific informatino to display 
		if (ImGui::Button("Entities", ImVec2(90, 25))) currentGUI = SHOW_GUI_ENTITIES;
		ImGui::SameLine();
		if (ImGui::Button("Lights", ImVec2(90, 25))) currentGUI = SHOW_GUI_LIGHTS;
		ImGui::SameLine();
		if (ImGui::Button("Cameras", ImVec2(90, 25))) currentGUI = SHOW_GUI_CAMERA;



		switch (currentGUI)
		{
		case SHOW_GUI_ENTITIES:	
			sceneGuis[currentScene]->UpdateEntityGUI(scenes[currentScene]->GetEntities());
			break;
		case SHOW_GUI_LIGHTS:
			sceneGuis[currentScene]->UpdateLightGUI(scenes[currentScene]->GetLights(), scenes[currentScene]->GetLightToGizmos());
			break;
		case SHOW_GUI_CAMERA:
			sceneGuis[currentScene]->UpdateCameraGUI(scenes[currentScene]->GetAllCams(), scenes[currentScene].get(), (float)this->windowWidth, (float)this->windowHeight);
			break;
		default:
			break;
		}


		ImGui::TreePop();
	}
	
	
	// Scene specific gui
	switch (currentScene)
	{
	case SCENE_ANIM:

		// Animation Scene 
		if (ImGui::TreeNode("Animation Controls"))
		{
			if (ImGui::Button("Animate", ImVec2(90, 25))) startAnimation = true;
			ImGui::Dummy(ImVec2(0, 10));

			ImGui::PushID(0);
			ImGui::Text("Seperation");
			animSceneGui->CreateCurveGuiWithDropDown("Seperation", &eyeSepCurve);
			ImGui::InputFloat("Animation Time", &eyeSepTime, 0.01f);
			ImGui::PopID();

			ImGui::PushID(1);
			ImGui::Text("Combine");
			animSceneGui->CreateCurveGuiWithDropDown("Combine", &eyeComCurve);
			ImGui::InputFloat("Animation Time", &eyeComTime, 0.01f);
			ImGui::PopID();

			ImGui::TreePop();
		}

		break;
	case SCENE_SHADOWS:
		shadowSceneGui->CreateShadowGui();
		break;
	default:
		break;
	}



	
	/*int type = sceneGui->CreateCurveGuiWithDropDown();*/
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	UpdateImGui(deltaTime);
	float mouseLookSpeed = 2.0f; 

	scene->GetCurrentCam()->Update(deltaTime);
	animManager->UpdateAnimations(deltaTime);

	switch (currentScene)
	{
	case SCENE_ANIM:
		AnimSceneLogic(deltaTime);
		break;
	default:
		break;
	}
	

	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
		Quit();
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erases what's on the screen)
		const float bgColor[4] = { 0.4f, 0.6f, 0.75f, 1.0f }; // Cornflower Blue
		context->ClearRenderTargetView(backBufferRTV.Get(), bgColor);

		// Clear the depth buffer (resets per-pixel occlusion information)
		context->ClearDepthStencilView(depthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}


	if (currentScene <= 0 || currentScene >= scenes.size())
	{
		// Default scene 
		/*scene->DrawShadows(
			context, 
			backBufferRTV,
			depthBufferDSV,
			shadowVS,
			(float)this->windowWidth,
			(float)this->windowHeight);*/
		scene->DrawEntities(context);
		scene->DrawLightsGui(context);
		scene->DrawSky(context);
	}
	else
	{
		switch (currentScene)
		{
		case SCENE_SHADOWS:
			scenes[currentScene]->DrawShadows(
				context,
				device,
				backBufferRTV,
				depthBufferDSV,
				shadowVS,
				SHADOW_MAP_RESOLUTION,
				(float)this->windowWidth,
				(float)this->windowHeight);
			{
				ID3D11ShaderResourceView* nullSRVs[128] = {};
				context->PSSetShaderResources(0, 128, nullSRVs);
			}
			
			break;
		default:
			break;
		}
		
		scenes[currentScene]->DrawEntities(context);
		scenes[currentScene]->DrawLightsGui(context);
		scenes[currentScene]->DrawSky(context);
	}

	
	
	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Present the back buffer to the user
		//  - Puts the results of what we've drawn onto the window
		//  - Without this, the user never sees anything
		bool vsyncNecessary = vsync || !deviceSupportsTearing || isFullscreen;


		// Draw ImGui
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());


		swapChain->Present(
			vsyncNecessary ? 1 : 0,
			vsyncNecessary ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Must re-bind buffers after presenting, as they become unbound
		context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
	}
}
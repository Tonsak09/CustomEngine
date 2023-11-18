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
		1280,				// Width of the window's client area
		720,				// Height of the window's client area
		false,				// Sync the framerate to the monitor refresh? (lock framerate)
		true)				// Show extra stats (fps) in title bar?
{
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
	scene = std::make_shared<Scene>();
	//entities = std::vector<std::shared_ptr<Entity>>();
	//cameras = std::vector<std::shared_ptr<Camera>>();
	//currentCam = 0;
	currentGUI = 0;

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
	std::vector<std::shared_ptr<Light>> lights = std::vector<std::shared_ptr<Light>>();

	directionalLight1 = {};
	directionalLight1.type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight1.directiton = DirectX::XMFLOAT3(1, -1, 0);
	directionalLight1.color = DirectX::XMFLOAT3(0.2f, 0.2f, 0.2f);
	directionalLight1.intensity = 1.0;
	lights.push_back(std::make_shared<Light>(directionalLight1));

	directionalLight2 = {};
	directionalLight2.type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight2.directiton = DirectX::XMFLOAT3(0, -1, 0);
	directionalLight2.color = DirectX::XMFLOAT3(1, 1, 1);
	directionalLight2.intensity = 1.0;
	lights.push_back(std::make_shared<Light>(directionalLight2));

	directionalLight3 = {};
	directionalLight3.type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight3.directiton = DirectX::XMFLOAT3(-0.2f, 1, 0);
	directionalLight3.color = DirectX::XMFLOAT3(0, 1, 0);
	directionalLight3.intensity = 1.0;
	lights.push_back(std::make_shared<Light>(directionalLight3));

	pointLight1 = {};
	pointLight1.type = LIGHT_TYPE_POINT;
	pointLight1.position = DirectX::XMFLOAT3(2, 2, 0);
	pointLight1.color = DirectX::XMFLOAT3(0, 1, 0);
	pointLight1.intensity = 1.0;
	pointLight1.range = 40.0;
	lights.push_back(std::make_shared<Light>(pointLight1));

	pointLight2 = {};
	pointLight2.type = LIGHT_TYPE_POINT;
	pointLight2.position = DirectX::XMFLOAT3(1, -3, 0);
	pointLight2.color = DirectX::XMFLOAT3(0, 0, 1);
	pointLight2.intensity = 1.0;
	pointLight2.range = 100.0;
	lights.push_back(std::make_shared<Light>(pointLight2));

	// Set the scene's lights 
	scene->SetLights(lights);
}

void Game::SetupLitMaterial(std::shared_ptr<Material> mat,
	const wchar_t albedoTextureAddress[],
	const wchar_t speculuarMapAddress[],
	const wchar_t normalMapAddress[],
	const char samplerType[])
{
	// Create the data storage struct 
	std::shared_ptr<MatData> tempData = std::make_shared<MatData>(device);
	matToResources[mat] = tempData;
	MatData* data = tempData.get();

	// Load in the textures 
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(albedoTextureAddress).c_str(), nullptr, data->albedo.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(speculuarMapAddress).c_str(), nullptr, data->spec.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(normalMapAddress).c_str(), nullptr, data->normal.GetAddressOf());

	// Apply to shader registers 
	mat.get()->AddSampler("BasicSampler", sampler);
	mat.get()->AddTextureSRV("SurfaceTexture", data->albedo);
	mat.get()->AddTextureSRV("NormalMap", data->normal);
	mat.get()->AddTextureSRV("SpeculuarTexture", data->spec);
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


	mat1 = std::make_shared<Material>(DirectX::XMFLOAT4(1, 1, 1, 1), 1.0f, DirectX::XMFLOAT2(0,0), vertexShader, customPShader);
	mat2 = std::make_shared<Material>(DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1), 1.0f, DirectX::XMFLOAT2(0, 0), vertexShader, pixelShader);
	mat3 = std::make_shared<Material>(DirectX::XMFLOAT4(1, 1, 0, 1), 1.0f, DirectX::XMFLOAT2(0, 0), vertexShader, pixelShader);
	lit = std::make_shared<Material>(DirectX::XMFLOAT4(1, 1, 1, 1), 0.5f, DirectX::XMFLOAT2(0, 0), vertexShader, litShader);
	//litCushion = std::make_shared<Material>(DirectX::XMFLOAT4(1, 1, 1, 1), 0.5f, DirectX::XMFLOAT2(0, 0), vertexShader, litShader);

	schlickBricks = std::make_shared<Material>(DirectX::XMFLOAT4(1, 1, 1, 1), 0.5f, DirectX::XMFLOAT2(0, 0), vertexShader, schlickShader);
	schlickCushions = std::make_shared<Material>(DirectX::XMFLOAT4(1, 1, 1, 1), 0.5f, DirectX::XMFLOAT2(0, 0), vertexShader, schlickShader);

	

	std::shared_ptr<Mesh> sphere = std::make_shared<Mesh>(device, context, FixPath(L"../../Assets/Models/sphere.obj").c_str());
	std::shared_ptr<Mesh> helix = std::make_shared<Mesh>(device, context, FixPath(L"../../Assets/Models/helix.obj").c_str());
	std::shared_ptr<Mesh> cube = std::make_shared<Mesh>(device, context, FixPath(L"../../Assets/Models/cube.obj").c_str());
	std::shared_ptr<Mesh> torus = std::make_shared<Mesh>(device, context, FixPath(L"../../Assets/Models/torus.obj").c_str());
	std::shared_ptr<Mesh> lightGUIModel = std::make_shared<Mesh>(device, context, FixPath(L"../../Assets/Models/LightGUIModel.obj").c_str());

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

	// Create materials 
	SetupLitMaterial(
		lit,
		L"../../Assets/Textures/rustymetal.png",
		L"../../Assets/Textures/rustymetal_specular.png",
		L"../../Assets/Textures/original.png"
	);

	SetupLitMaterial(
		schlickBricks,
		L"../../Assets/Textures/ass9/cobblestone.png",
		L"../../Assets/Textures/rustymetal_specular.png",
		L"../../Assets/Textures/ass9/cobblestone_normals.png"
	);
	schlickBricks->AddTextureSRV("Environment", sky->GetCubeSRV());

	SetupLitMaterial(
		schlickCushions,
		L"../../Assets/Textures/ass9/cushion.png",
		L"../../Assets/Textures/rustymetal_specular.png",
		L"../../Assets/Textures/ass9/cushion_normals.png"
	);
	schlickCushions->AddTextureSRV("Environment", sky->GetCubeSRV());
	
	std::vector<std::shared_ptr<Entity>> entities = std::vector<std::shared_ptr<Entity>>();

	// Add all entites to the primary vector 
	entities.push_back(std::shared_ptr<Entity>(new Entity(helix, lit)));
	entities[0]->GetTransform()->SetPosition(1.0f, 0.0f, 0.0f);

	entities.push_back(std::shared_ptr<Entity>(new Entity(sphere, schlickBricks)));
	entities[1]->GetTransform()->MoveRelative(5.0f, 0.0f, 0.0f);

	entities.push_back(std::shared_ptr<Entity>(new Entity(cube, schlickCushions)));
	entities[2]->GetTransform()->MoveRelative(-5.0f, 0.0f, 0.0f);

	scene->SetEntities(entities);
	scene->GenerateLightGizmos(lightGUIModel, vertexShader, pixelShader);
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

	scene->ResizeCam((float)this->windowWidth, this->windowHeight);
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

	// Buttons 
	if (ImGui::Button("Entities", ImVec2(90, 25))) currentGUI = SHOW_GUI_ENTITIES;
	ImGui::SameLine();
	if (ImGui::Button("Lights", ImVec2(90, 25))) currentGUI = SHOW_GUI_LIGHTS;
	ImGui::SameLine();
	if (ImGui::Button("Camera", ImVec2(90, 25))) currentGUI = SHOW_GUI_CAMERA;


	switch (currentGUI)
	{
	case SHOW_GUI_ENTITIES:
		sceneGui->UpdateEntityGUI(scene->GetEntities());
		break;
	case SHOW_GUI_LIGHTS:
		sceneGui->UpdateLightGUI(scene->GetLights(), scene->GetLightToGizmos());
		break;
	case SHOW_GUI_CAMERA:
		sceneGui->UpdateCameraGUI(scene->GetAllCams(), scene.get(), (float)this->windowWidth, (float)this->windowHeight);
		break;
	default:
		break;
	}

	int type = sceneGui->CreateCurveGuiWithDropDown();
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	UpdateImGui(deltaTime);
	float mouseLookSpeed = 2.0f; 

	scene->GetCurrentCam()->Update(deltaTime);

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

	scene->DrawEntities(context);
	scene->DrawLightsGui(context);
	scene->DrawSky(context);
	
	//for (unsigned int i = 0; i < entities.size(); i++)
	//{
	//	DirectX::XMFLOAT3 ambient(0.1f, 0.1f, 0.25f);
	//	entities[i]->GetMat()->GetPixelShader()->SetFloat3("ambient", ambient);

	//	int dLights = 1; 
	//	int sLights = 1;
	//	int pLights = 1;
	//	for (int l = 0; l < lights.size(); l++)
	//	{
	//		int lightType = lights[l].type;
	//		std::string name; //= (lights[l].type == 0 ? "directionalLight" : "spotLight") + std::to_string(l + 1);

	//		switch (lightType)
	//		{
	//		case LIGHT_TYPE_DIRECTIONAL:
	//			name = "directionalLight" + std::to_string(dLights);
	//			dLights++;
	//			break;
	//		case  LIGHT_TYPE_POINT :
	//			name = "pointLight" + std::to_string(pLights);
	//			pLights++;
	//			break;
	//		case LIGHT_TYPE_SPOT: // Not implemented yet 
	//		default:
	//			continue;
	//		}

	//		entities[i]->GetMat()->GetPixelShader()->SetData(
	//			name, // The name of the (eventual) variable in the shader
	//			&lights[l], // The address of the data to set
	//			sizeof(Light)); // The size of the data (the whole struct!) to set
	//	}



	//	entities[i]->Draw(context, cameras[currentCam]);
	//}

	/*for (unsigned int i = 0; i < lightGizmos.size(); i++)
	{
		lightGizmos[i]->Draw(context, cameras[currentCam]);
	}*/

	//sky->Draw(cameras[currentCam]);

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
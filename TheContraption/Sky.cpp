#include "Sky.h"
#include <WICTextureLoader.h>
#include "DDSTextureLoader.h"

using namespace DirectX;

Sky::Sky(Microsoft::WRL::ComPtr<ID3D11Device> device, 
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
    Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler, 
    std::shared_ptr<Mesh> mesh, 
    const wchar_t right[], 
    const wchar_t left[], 
    const wchar_t up[], 
    const wchar_t down[], 
    const wchar_t front[], 
    const wchar_t back[],
    const wchar_t pixelShaderPath[],
    const wchar_t vertexShaderPath[]) :
    device(device), context(context), sampler(sampler), mesh(mesh)
{
    cubeSRV = CreateCubemap(
        right,
        left,
        up,
        down,
        front,
        back
    );

    // Load Shaders
    skyPS = std::make_shared<SimplePixelShader>(device, context,
        FixPath(pixelShaderPath).c_str());
    skyVS = std::make_shared<SimpleVertexShader>(device, context,
        FixPath(vertexShaderPath).c_str());

    // Load States
    D3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.CullMode = D3D11_CULL_FRONT;
    rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.DepthClipEnable = true;
    device->CreateRasterizerState(&rastDesc, rasterizeState.GetAddressOf());

    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = true;
    depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL; 
    depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    device->CreateDepthStencilState(&depthDesc, stencilState.GetAddressOf());
}

void Sky::Draw(std::shared_ptr<Camera> cam)
{
    context->OMSetDepthStencilState(stencilState.Get(), 0);
    context->RSSetState(rasterizeState.Get());

    // Setting 
    skyVS->SetShader();
    skyPS->SetShader();

    // Vertex Data
    skyVS->SetMatrix4x4("view", *cam->GetViewMatrix());
    skyVS->SetMatrix4x4("proj", *cam->GetProjMatrix());
    skyVS->CopyAllBufferData();

    // Pixel Data
    skyPS->SetShaderResourceView("CubeMap", cubeSRV);
    skyPS->SetSamplerState("BasicSampler", sampler);

    // Finally draw
    mesh->Draw();

    // Reset everything so settings to stay for other draws 
    context->RSSetState(0);
    context->OMSetDepthStencilState(0, 0);
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Sky::GetCubeSRV()
{
    return cubeSRV;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Sky::CreateCubemap(const wchar_t* right, const wchar_t* left, const wchar_t* up, const wchar_t* down, const wchar_t* front, const wchar_t* back)
{
    // Load the 6 textures into an array.
// - We need references to the TEXTURES, not SHADER RESOURCE VIEWS!
// - Explicitly NOT generating mipmaps, as we don't need them for the sky!
// - Order matters here! +X, -X, +Y, -Y, +Z, -Z
    Microsoft::WRL::ComPtr<ID3D11Texture2D> textures[6] = {};
    CreateWICTextureFromFile(device.Get(), FixPath(right).c_str(), (ID3D11Resource**)textures[0].GetAddressOf(), 0);
    CreateWICTextureFromFile(device.Get(), FixPath(left).c_str(), (ID3D11Resource**)textures[1].GetAddressOf(), 0);
    CreateWICTextureFromFile(device.Get(), FixPath(up).c_str(), (ID3D11Resource**)textures[2].GetAddressOf(), 0);
    CreateWICTextureFromFile(device.Get(), FixPath(down).c_str(), (ID3D11Resource**)textures[3].GetAddressOf(), 0);
    CreateWICTextureFromFile(device.Get(), FixPath(front).c_str(), (ID3D11Resource**)textures[4].GetAddressOf(), 0);
    CreateWICTextureFromFile(device.Get(), FixPath(back).c_str(), (ID3D11Resource**)textures[5].GetAddressOf(), 0);
    // We'll assume all of the textures are the same color format and resolution,
    // so get the description of the first texture
    D3D11_TEXTURE2D_DESC faceDesc = {};
    textures[0]->GetDesc(&faceDesc);
    // Describe the resource for the cube map, which is simply
    // a "texture 2d array" with the TEXTURECUBE flag set.
    // This is a special GPU resource format, NOT just a
    // C++ array of textures!!!
    D3D11_TEXTURE2D_DESC cubeDesc = {};
    cubeDesc.ArraySize = 6; // Cube map!
    cubeDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // We'll be using as a texture in a shader
    cubeDesc.CPUAccessFlags = 0; // No read back
    cubeDesc.Format = faceDesc.Format; // Match the loaded texture's color format
    cubeDesc.Width = faceDesc.Width; // Match the size
    cubeDesc.Height = faceDesc.Height; // Match the size
    cubeDesc.MipLevels = 1; // Only need 1
    cubeDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE; // A CUBE, not 6 separate textures
    cubeDesc.Usage = D3D11_USAGE_DEFAULT; // Standard usage
    cubeDesc.SampleDesc.Count = 1;
    cubeDesc.SampleDesc.Quality = 0;
    // Create the final texture resource to hold the cube map
    Microsoft::WRL::ComPtr<ID3D11Texture2D> cubeMapTexture;
    device->CreateTexture2D(&cubeDesc, 0, cubeMapTexture.GetAddressOf());
    // Loop through the individual face textures and copy them,
    // one at a time, to the cube map texure
    for (int i = 0; i < 6; i++)
    {
        // Calculate the subresource position to copy into
        unsigned int subresource = D3D11CalcSubresource(
            0, // Which mip (zero, since there's only one)
            i, // Which array element?
            1); // How many mip levels are in the texture?
        // Copy from one resource (texture) to another
        context->CopySubresourceRegion(
            cubeMapTexture.Get(), // Destination resource
            subresource, // Dest subresource index (one of the array elements)
            0, 0, 0, // XYZ location of copy
            textures[i].Get(), // Source resource
            0, // Source subresource index (we're assuming there's only one)
            0); // Source subresource "box" of data to copy (zero means the whole thing)
    }
    // At this point, all of the faces have been copied into the
    // cube map texture, so we can describe a shader resource view for it
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = cubeDesc.Format; // Same format as texture
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE; // Treat this as a cube!
    srvDesc.TextureCube.MipLevels = 1; // Only need access to 1 mip
    srvDesc.TextureCube.MostDetailedMip = 0; // Index of the first mip we want to see
    // Make the SRV
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubeSRV;
    device->CreateShaderResourceView(cubeMapTexture.Get(), &srvDesc, cubeSRV.GetAddressOf());
    // Send back the SRV, which is what we need for our shaders
    return cubeSRV;
}

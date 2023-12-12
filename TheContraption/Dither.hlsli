#include "ShaderInclude.hlsli"

void DitherLogic(
VertexToPixel input,        // Info frrom Vertex Shader 
Texture2D Dither,           // Textures to for dither patterrns 
SamplerState BasicSampler,  // Allows us to sample our texture (We do this here since we don't know which dither texture) 
float aspect,               // Aspect ratio of screen 
float ditherLevel           // DEBUG VALUE 
)
{
	// Screenspace 
    float2 textureCoordinate = input.screenPos.xy / input.screenPos.w;
    textureCoordinate.x = textureCoordinate.x * aspect;

    float ditherCull = lerp(1, Dither.Sample(BasicSampler, textureCoordinate * 20.0f).x, ditherLevel);
    if (ditherCull < 0.9f) // Not within range 
    {
        discard;
    }
}
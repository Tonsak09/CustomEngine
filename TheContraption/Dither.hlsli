#include "ShaderInclude.hlsli"

void DitherLogic(
VertexToPixel input, 
Texture2D Dither, 
SamplerState BasicSampler, 
float attenuate, 
float aspect, 
float ditherLevel
)
{
	// Screenspace 
    float2 textureCoordinate = input.screenPos.xy / input.screenPos.w;
    textureCoordinate.x = textureCoordinate.x * aspect;

    float ditherCull = lerp(1, Dither.Sample(BasicSampler, textureCoordinate * 20.0f).x, ditherLevel);
    float cullByAttenuate = lerp(1, ditherCull, attenuate);
    if (ditherCull < 0.9f) // Not within range 
    {
        discard;
    }
	//else if(cullByAttenuate < )
}
#include "ShaderInclude.hlsli"


cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	float time;
}



// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	float rA = gnoise(input.uv + float2(time, 0));
	float rB = gnoise(input.uv + float2(time / 0.5, -time));

	float col = rA + rB - 0.5f;
	return float4(col, col, col, 1.0);
}
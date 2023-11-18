#include "ShaderInclude.hlsli"

cbuffer ExternalData : register(b0)
{
	matrix view;
	matrix proj;
}



VertexToPixel_Sky main(VertexShaderInput input)
{
	VertexToPixel_Sky output;

	matrix viewNoTranslation = view;
	viewNoTranslation._14 = 0;
	viewNoTranslation._24 = 0;
	viewNoTranslation._34 = 0;

	matrix vp = mul(proj, viewNoTranslation);
	output.position = mul(vp, float4(input.localPosition, 1.0f));

	// Set to limit 
	output.position.z = output.position.w;

	// Apply direction as "3D uv"
	output.sampleDir = input.localPosition;

	return output;
}
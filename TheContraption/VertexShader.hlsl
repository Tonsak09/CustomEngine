#include "ShaderInclude.hlsli"

// Allows us to send in data through C++ 
cbuffer ExternalData : register(b0)
{
	// We are defining the order that these data structures
	// are being stored within the shader

	// The data order does matter! Smaller data being placed first will
	// cause there to be spacing to be added by direct without telling us 
	// Since we need to know exactly how long items being passed are into
	// the shader this can cause a large issue for us.
	matrix world; // Equivelent to 4x4 
	matrix viewMatrix;
	matrix projMatrix;
	matrix worldInvTranspose;
	matrix lightView;
	matrix lightProjection;
}


// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main( VertexShaderInput input )
{
	VertexToPixel output;
	
	// Multiply the three matrices together first
	matrix mvp = mul(projMatrix, mul(viewMatrix, world));
	output.screenPosition = mul(mvp, float4(input.localPosition, 1.0f));

	output.uv = input.uv;

	output.normal = mul((float3x3)worldInvTranspose, input.normal); // Perfect
	output.tangent = mul((float3x3)world, input.tangent);
	output.worldPosition = mul(world, float4(input.localPosition, 1.0f)).xyz;
	
	float4 clip = mul(mvp, float4(input.localPosition, 1.0f));  // into clip space


	//xNDC *= 720;
	//yNDC *= 720;
	output.screenPos = clip;
	//output.screenPos = float4(xNDC, yNDC, 0, 1); //mul(mul(viewMatrix, input.localPosition), clip);  // into clip space


	// Shadows 
	matrix shadowMvp = mul(lightProjection, mul(lightView, world));
	output.shadowMapPos = mul(shadowMvp, float4(input.localPosition, 1.0f));

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}
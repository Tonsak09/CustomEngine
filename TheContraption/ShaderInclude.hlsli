#ifndef __GGP_SHADER_INCLUDES__ // Each .hlsli file needs a unique identifier!
#define __GGP_SHADER_INCLUDES__


#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

struct Light
{
	int type;
	float3 directiton;
	float range;
	float3 position;
	float intensity;
	float3 color;
	float spotFalloff;
	float3 padding;
};

// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float3 localPosition	: POSITION;     // XYZ position
	float3 normal			: NORMAL;
	float3 tangent			: TANGENT;
	float4 uv				: TEXCOORD;
};

// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 screenPosition	: SV_POSITION;
	float3 worldPosition	: POSITION;
	float2 uv				: TEXCOORD;
	float3 normal			: NORMAL;
	float3 tangent			: TANGENT;
};

struct VertexToPixel_Sky
{
	float4 position		: SV_POSITION;
	float3 sampleDir	: DIRECTION;
};

float rand2(float2 n) { return frac(sin(dot(n, float2(12.9898, 4.1414))) * 43758.5453); }


float gnoise(float2 n) {
	const float2 d = float2(0.0, 1.0);
	float2  b = floor(n),
		f = smoothstep(d.xx, d.yy, frac(n));

	//float2 f = frac(n);
	//f = f*f*(3.0-2.0*f);

	float x = lerp(rand2(b), rand2(b + d.yx), f.x),
		y = lerp(rand2(b + d.xy), rand2(b + d.yy), f.x);

	return lerp(x, y, f.y);
}

float SimpleFresnel(float3 n, float3 v, float f0)
{
	// Pre-calculations
	float NdotV = saturate(dot(n, v));

	// Final value
	return f0 + (1 - f0) * pow(1 - NdotV, 5);
}


#endif
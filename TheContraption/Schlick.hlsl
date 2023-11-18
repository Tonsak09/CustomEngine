

#define MAX_SPECULAR_EXPONENT 256.0f

#include "ShaderInclude.hlsli"

Texture2D SurfaceTexture : register(t0); // "t" registers for textures
Texture2D SpeculuarTexture : register(t1); // "t" registers for textures

Texture2D NormalMap : register(t2);
TextureCube Environment : register(t3);
SamplerState BasicSampler : register(s0); // "s" registers for samplers

cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	float3 camPos;
	float roughness;
	float2 uvOffset;
	float3 ambient;
	Light directionalLight1;
	Light directionalLight2;
	Light directionalLight3;
	Light pointLight1;
	Light pointLight2;
}


float2 GetUV(VertexToPixel input)
{
	return input.uv + uvOffset;
}

float3 GetSurfaceColor(VertexToPixel input)
{
	float3 surfaceColor = SurfaceTexture.Sample(BasicSampler, GetUV(input)).rgb;
	return surfaceColor;
}

float GetSpec(VertexToPixel input)
{
	return SpeculuarTexture.Sample(BasicSampler, GetUV(input)).r;
}

float Attenuate(Light light, float3 worldPos)
{
	float dist = distance(light.position, worldPos);
	float att = saturate(1.0f - (dist * dist / (light.range * light.range)));
	return att * att;
}

float3 DirLight(Light light, VertexToPixel input, float3 ambient, float roughness)
{
	float3 lightDir = normalize(-light.directiton);
	float3 diffuse = saturate(dot(input.normal, lightDir));

	float3 diffColor = (diffuse * light.color * GetSurfaceColor(input)) + (ambient * GetSurfaceColor(input));

	float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
	float3 V = normalize(input.worldPosition - camPos);
	float3 R = reflect(lightDir, input.normal);

	float spec = pow(saturate(dot(R, V)), specExponent) * GetSpec(input) * any(diffuse);

	return (diffColor * diffuse + spec);
}

float3 PointLight(Light light, VertexToPixel input, float3 ambient, float roughness)
{
	float3 lightDir = normalize(light.position - input.worldPosition);
	float3 diffuse = saturate(dot(input.normal, lightDir));

	float3 diffColor = (diffuse * light.color * GetSurfaceColor(input)) + (ambient * GetSurfaceColor(input));

	float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
	float3 V = normalize(input.worldPosition - camPos);
	float3 R = reflect(lightDir, input.normal);

	float spec = pow(saturate(dot(R, V)), specExponent) * GetSpec(input) * any(diffuse);

	return (diffColor * diffuse + spec) * Attenuate(light, input.worldPosition);
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
	float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1;
	unpackedNormal = normalize(unpackedNormal); // Don’t forget to normalize!


	// Simplifications include not re-normalizing the same vector more than once!
	float3 N = normalize(input.normal); // Must be normalized here or before
	float3 T = normalize(input.tangent); // Must be normalized here or before
	T = normalize(T - N * dot(T, N)); // Gram-Schmidt assumes T&N are normalized!
	float3 B = cross(T, N);
	float3x3 TBN = float3x3(T, B, N);

	// Assumes that input.normal is the normal later in the shader
	input.normal = mul(unpackedNormal, TBN); // Note multiplication order!


	// Dir lights 
	float3 light1 = DirLight(directionalLight1, input, ambient, roughness);
	float3 light2 = DirLight(directionalLight2, input, ambient, roughness);
	float3 light3 = DirLight(directionalLight3, input, ambient, roughness);

	// Point lights
	float3 light4 = PointLight(pointLight1, input, ambient, roughness);
	float3 light5 = PointLight(pointLight2, input, ambient, roughness);

	float3 totalLight = light1 + light2 + light3 + light4 + light5;


	// Environment
	float3 viewVector = normalize(camPos - input.worldPosition);
	float3 reflectionVector = reflect(-viewVector, input.normal); // Need camera to pixel vector, so negate
	float3 reflectionColor = Environment.Sample(BasicSampler, reflectionVector).rgb;

	// 0.04f is recommended amount 
	float3 finalColor = lerp(totalLight, reflectionColor, SimpleFresnel(input.normal, viewVector, 0.04f));

	return float4(finalColor, 1);
}


#define MAX_SPECULAR_EXPONENT 256.0f

#include "ShaderInclude.hlsli"
#include "PBRFunctions.hlsli"

// TODO:	Pass in variables that need to be read by sampler
//			so that we do not read more than once

Texture2D Albedo : register(t0); 
Texture2D NormalMap : register(t1);
Texture2D RoughnessMap : register(t2);
Texture2D MetalnessMap : register(t3);
TextureCube Environment : register(t4);
Texture2D ShadowMap : register(t5); 

SamplerState BasicSampler : register(s0);


cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	float3 camPos;
	float2 uvOffset;
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

float4 GetAlbedo(VertexToPixel input)
{
	float4 surfaceColor = Albedo.Sample(BasicSampler, GetUV(input));
	return surfaceColor;//pow(surfaceColor.rgb, 2.2f);
}

float GetRoughness(VertexToPixel input)
{
	//return 0.5;
	return RoughnessMap.Sample(BasicSampler, input.uv).r;
}

float GetMetalness(VertexToPixel input)
{
	//return 1.0f;
	return MetalnessMap.Sample(BasicSampler, input.uv).r;
}

float3 GetSpec(VertexToPixel input)
{
	return lerp(F0_NON_METAL, GetAlbedo(input).rgb, GetMetalness(input));
}

/// <summary>
/// Lowers light strength over distance 
/// </summary>
float Attenuate(Light light, float3 worldPos)
{
	float dist = distance(light.position, worldPos);
	float att = saturate(1.0f - (dist * dist / (light.range * light.range)));
	return att * att;
}

float3 DirLight(Light light, VertexToPixel input, float roughness, float metalness, float3 albedo, float3 specColor)
{
	float3 lightDir = normalize(-light.directiton);
	float3 V = normalize(camPos - input.worldPosition);
	

	//float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
	float3 R = reflect(input.normal, lightDir);

	float3 diffuse = saturate(dot(input.normal, lightDir));
	float3 F;
	float3 spec = MicrofacetBRDF(input.normal, lightDir, V, roughness, specColor, F);

	// Calculate diffuse with energy conservation, including cutting diffuse for metals
	float3 balancedDiff = DiffuseEnergyConserve(diffuse, spec, metalness);
	// Combine the final diffuse and specular values for this light
	float3 total = (balancedDiff * albedo + spec) * light.intensity * light.color;


	return total;//(diffColor * diffuse + spec);
}

float3 PointLight(Light light, VertexToPixel input, float roughness, float metalness, float3 albedo, float3 specColor)
{
	float3 difference = light.position - input.worldPosition;
	float mag = length(difference);

	if (mag > light.range)
		return float3(0, 0, 0);

	float3 lightDir = normalize(light.position - input.worldPosition);
	float3 V = normalize(camPos - input.worldPosition);

	float atten = Attenuate(light, input.worldPosition);
	float3 diffuse = saturate(dot(input.normal, lightDir));
	float3 F;
	float3 spec = MicrofacetBRDF(input.normal, lightDir, V, roughness, specColor, F);
	
	float3 balancedDiff = DiffuseEnergyConserve(diffuse, spec, metalness);

	float3 total = (balancedDiff * albedo + spec) * light.intensity * light.color;



	return total;
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
	// Sample textures   
	float4 albedo = GetAlbedo(input);
	
	if (albedo.a < 0.1)
	{
		discard;
	}

	// Perform the perspective divide (divide by W) ourselves
	input.shadowMapPos /= input.shadowMapPos.w;
	// Convert the normalized device coordinates to UVs for sampling
	float2 shadowUV = input.shadowMapPos.xy * 0.5f + 0.5f;
	shadowUV.y = 1 - shadowUV.y; // Flip the Y
	// Grab the distances we need: light-to-pixel and closest-surface
	float distToLight = input.shadowMapPos.z;
	float distShadowMap = ShadowMap.Sample(BasicSampler, shadowUV).r;
	// For testing, just return black where there are shadows.
	if (distShadowMap < distToLight)
		return float4(0, 0, 0, 1);


	float roughness = GetRoughness(input);
	float metalness = GetMetalness(input);
	float3 specColor = GetSpec(input);


	// Normals 
	float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1;
	unpackedNormal = normalize(unpackedNormal);

	// Simplifications include not re-normalizing the same vector more than once!
	float3 N = normalize(input.normal); // Must be normalized here or before
	float3 T = normalize(input.tangent); // Must be normalized here or before
	T = normalize(T - N * dot(T, N)); // Gram-Schmidt assumes T&N are normalized!
	float3 B = cross(T, N);
	float3x3 TBN = float3x3(T, B, N);

	// Assumes that input.normal is the normal later in the shader
	input.normal = mul(unpackedNormal, TBN); // Note multiplication order!

	// Dir lights 
	float3 light1 = DirLight(directionalLight1, input, roughness, metalness, albedo, specColor);
	float3 light2 = DirLight(directionalLight2, input, roughness, metalness, albedo, specColor);
	float3 light3 = DirLight(directionalLight3, input, roughness, metalness, albedo, specColor);

	// Point lights
	float3 light4 = PointLight(pointLight1, input, roughness, metalness, albedo, specColor);
	float3 light5 = PointLight(pointLight2, input, roughness, metalness, albedo, specColor);

	float3 totalLight = light1 + light2 + light3 + light4 + light5;


	// Environment
	float3 viewVector = normalize(camPos - input.worldPosition);
	float3 reflectionVector = reflect(-viewVector, input.normal); // Need camera to pixel vector, so negate
	float3 reflectionColor = Environment.Sample(BasicSampler, reflectionVector).rgb;
	
	// 0.04f is recommended amount 
	float3 finalColor = lerp(totalLight, reflectionColor, SimpleFresnel(input.normal, viewVector, 0.04f));

	//return float4(input.uv, 0, 1);
	return float4( pow(finalColor, 1.0f / 2.2f), 1); //float4(pow(finalColor, 1.0f / 2.2f), 1);
}
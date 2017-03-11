struct DirectionalLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Direction;
	float pad;
};

cbuffer cbPerFrame : register(b0)
{
	DirectionalLight gDirLight;
}

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 NormalWorld : NORMAL;
};

void ComputeDirectionalLight(float3 normal, DirectionalLight light, out float4 diffuse)
{
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightVec = -light.Direction;

	float diffuseFactor = dot(lightVec, normal);

	[flatten]
	if (diffuseFactor > 0.0f)
	{
		diffuse = float4(diffuseFactor, diffuseFactor, diffuseFactor, 1.0f);
	}
}


float4 main(VertexOut vin) : SV_TARGET
{
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
	ComputeDirectionalLight(vin.NormalWorld, gDirLight, diffuse);

	return diffuse;
}
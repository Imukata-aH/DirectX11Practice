struct DirectionalLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Direction;
	float pad;
};

struct Material
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular; // w = SpecPower
	float4 Reflect;
};

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld;
	float4x4 gWorldViewProj;
	Material gMaterial;
};

cbuffer cbPerFrame : register(b1)
{
	DirectionalLight gDirLight;
	float3 gEyePosW;
}

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 NormalWorld : NORMAL;
};

void ComputeDirectionalLight(float3 normal, DirectionalLight light, Material mat, float3 eyePos, out float4 ambient, out float4 diffuse, out float4 specular)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightVec = -light.Direction;

	ambient = mat.Ambient * light.Ambient;

	float diffuseFactor = dot(lightVec, normal);

	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, eyePos), 0.0f), mat.Specular.w);

		specular = specFactor * mat.Specular * light.Specular;
		diffuse = diffuseFactor * mat.Diffuse * light.Diffuse;
	}
}


float4 main(VertexOut vin) : SV_TARGET
{
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
	ComputeDirectionalLight(vin.NormalWorld, gDirLight, gMaterial, gEyePosW, ambient, diffuse, specular);

	return ambient + diffuse + specular;
}
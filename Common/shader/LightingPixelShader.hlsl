#include "LightHelper.h"

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
	Material gMaterial;
};

cbuffer cbPerFrame : register(b1)
{
	DirectionalLight gDirLight;
	PointLight gPointLight;
	SpotLight gSpotLight;
	float3 gEyePosW;
}

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 PosW : POSITION;
	float3 NormalW : NORMAL;
};

float4 main(VertexOut pin) : SV_TARGET
{
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
	float3 toEyeW = normalize(gEyePosW - pin.PosW);

	float4 A, D, S;

	ComputeDirectionalLight(pin.NormalW, gDirLight, gMaterial, toEyeW, A, D, S);
	ambient += A;
	diffuse += D;
	specular += S;

	ComputePointLight(pin.NormalW, gPointLight, gMaterial, pin.PosW, toEyeW, A, D, S);
	ambient += A;
	diffuse += D;
	specular += S;

	ComputeSpotLight(pin.NormalW, gSpotLight, gMaterial, pin.PosW, toEyeW, A, D, S);
	ambient += A;
	diffuse += D;
	specular += S;

	return ambient + diffuse + specular;
}
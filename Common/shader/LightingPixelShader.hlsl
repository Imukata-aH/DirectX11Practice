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
	DirectionalLight gDirLight[3];
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

	[unroll]
	for (int i = 0; i < 3; i++)
	{
		float4 A, D, S;
		ComputeDirectionalLight(pin.NormalW, gDirLight[i], gMaterial, toEyeW, A, D, S);
		ambient += A;
		diffuse += D;
		specular += S;
	}


	return ambient + diffuse + specular;
}
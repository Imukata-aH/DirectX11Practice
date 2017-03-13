#include "LightHelper.h"

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
	Material gMaterial;
};

struct VertexIn
{
	float3 Pos : POSITION;
	float3 NormalL : NORMAL;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 PosW : POSITION;
	float3 NormalW : NORMAL;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;

	//transform to homogenous clip space 
	vout.PosH = mul(float4(vin.Pos, 1.0f), gWorldViewProj);

	vout.PosW = mul(float4(vin.Pos, 1.0f), gWorld).xyz;

	vout.NormalW = mul(vin.NormalL, (float3x3) gWorldInvTranspose);
	vout.NormalW = normalize(vout.NormalW);

	return vout;
}
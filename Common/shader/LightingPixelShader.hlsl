cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 Normal : NORMAL;
};

float4 main(VertexOut vin) : SV_TARGET
{
	return float4(vin.Normal, 1.0f);
}
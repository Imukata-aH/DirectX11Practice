cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
};

struct VertexIn
{
	float3 Pos : POSITION;
	float3 Normal : NORMAL;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 Normal : NORMAL;
};

VertexOut main(VertexIn vin)
{
	//transform to homogenous clip space 
	VertexOut vout;
	vout.PosH = mul(float4(vin.Pos, 1.0f), gWorldViewProj);
	vout.Normal = mul(float4(vin.Normal, 1.0f), gWorldViewProj).xyz;
	return vout;
}
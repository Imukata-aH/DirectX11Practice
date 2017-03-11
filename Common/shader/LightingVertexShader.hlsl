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

struct VertexIn
{
	float3 Pos : POSITION;
	float3 NormalLocal : NORMAL;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 NormalWorld : NORMAL;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;

	//transform to homogenous clip space 
	vout.PosH = mul(float4(vin.Pos, 1.0f), gWorldViewProj);

	vout.NormalWorld = mul(vin.NormalLocal, (float3x3)gWorld);
	vout.NormalWorld = normalize(vout.NormalWorld);

	return vout;
}
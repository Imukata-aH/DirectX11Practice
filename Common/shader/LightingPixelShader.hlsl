struct DirectionalLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Direction;
	float pad;
};

struct PointLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	float3 Position;
	float Range;

	float3 Attenuation;
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
	PointLight gPointLight;
	float3 gEyePosW;
}

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 PosW : POSITION;
	float3 NormalW : NORMAL;
};

void ComputeDirectionalLight(float3 normal, DirectionalLight light, Material mat, float3 toEye, out float4 ambient, out float4 diffuse, out float4 specular)
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
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

		specular = specFactor * mat.Specular * light.Specular;
		diffuse = diffuseFactor * mat.Diffuse * light.Diffuse;
	}
}

void ComputePointLight(float3 normal, PointLight light, Material mat, float3 pos, float3 toEye, out float4 ambient, out float4 diffuse, out float4 specular)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightVec = light.Position - pos;

	float d = length(lightVec);

	if(d > light.Range)
		return;

	// normalize
	lightVec /= d;

	ambient = mat.Ambient * light.Ambient;

	float diffuseFactor = dot(lightVec, normal);

	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

		specular = specFactor * mat.Specular * light.Specular;
		diffuse = diffuseFactor * mat.Diffuse * light.Diffuse;
	}

	// Attenuation
	float att = 1.0f / (dot(light.Attenuation, float3(1.0f, d, d * d)));

	diffuse *= att;
	specular *= att;
}

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

	return ambient + diffuse + specular;
}
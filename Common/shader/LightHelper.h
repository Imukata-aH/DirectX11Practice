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

struct SpotLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	float3 Position;
	float Range;

	float3 Direction;
	float Spot;

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

void ComputeDirectionalLight( float3 normal, DirectionalLight light, Material mat, float3 toEye, out float4 ambient, out float4 diffuse, out float4 specular )
{
	ambient = float4( 0.0f, 0.0f, 0.0f, 0.0f );
	diffuse = float4( 0.0f, 0.0f, 0.0f, 0.0f );
	specular = float4( 0.0f, 0.0f, 0.0f, 0.0f );

	float3 lightVec = -light.Direction;

	ambient = mat.Ambient * light.Ambient;

	float diffuseFactor = dot( lightVec, normal );

	[flatten]
	if ( diffuseFactor > 0.0f )
	{
		float3 v = reflect( -lightVec, normal );
		float specFactor = pow( max( dot( v, toEye ), 0.0f ), mat.Specular.w );

		specular = specFactor * mat.Specular * light.Specular;
		diffuse = diffuseFactor * mat.Diffuse * light.Diffuse;
	}
}

void ComputePointLight( float3 normal, PointLight light, Material mat, float3 pos, float3 toEye, out float4 ambient, out float4 diffuse, out float4 specular )
{
	ambient = float4( 0.0f, 0.0f, 0.0f, 0.0f );
	diffuse = float4( 0.0f, 0.0f, 0.0f, 0.0f );
	specular = float4( 0.0f, 0.0f, 0.0f, 0.0f );

	float3 lightVec = light.Position - pos;

	float d = length( lightVec );

	if ( d > light.Range )
		return;

	// normalize
	lightVec /= d;

	ambient = mat.Ambient * light.Ambient;

	float diffuseFactor = dot( lightVec, normal );

	[flatten]
	if ( diffuseFactor > 0.0f )
	{
		float3 v = reflect( -lightVec, normal );
		float specFactor = pow( max( dot( v, toEye ), 0.0f ), mat.Specular.w );

		specular = specFactor * mat.Specular * light.Specular;
		diffuse = diffuseFactor * mat.Diffuse * light.Diffuse;
	}

	// Attenuation
	float att = 1.0f / ( dot( light.Attenuation, float3( 1.0f, d, d * d ) ) );

	diffuse *= att;
	specular *= att;
}

void ComputeSpotLight( float3 normal, SpotLight light, Material mat, float3 pos, float3 toEye, out float4 ambient, out float4 diffuse, out float4 specular )
{
	ambient = float4( 0.0f, 0.0f, 0.0f, 0.0f );
	diffuse = float4( 0.0f, 0.0f, 0.0f, 0.0f );
	specular = float4( 0.0f, 0.0f, 0.0f, 0.0f );

	float3 lightVec = light.Position - pos;

	float d = length( lightVec );

	if ( d > light.Range )
		return;

	// normalize
	lightVec /= d;

	ambient = mat.Ambient * light.Ambient;

	float diffuseFactor = dot( lightVec, normal );

	[flatten]
	if ( diffuseFactor > 0.0f )
	{
		float3 v = reflect( -lightVec, normal );
		float specFactor = pow( max( dot( v, toEye ), 0.0f ), mat.Specular.w );

		specular = specFactor * mat.Specular * light.Specular;
		diffuse = diffuseFactor * mat.Diffuse * light.Diffuse;
	}

	// Scale by spotlight factor and attenuate.
	float spot = pow( max( dot( -lightVec, light.Direction ), 0.0f ), light.Spot );
	float att = spot / ( dot( light.Attenuation, float3( 1.0f, d, d * d ) ) );

	ambient *= spot;
	diffuse *= att;
	specular *= att;
}
#include "Effects.h"
#include "ShaderHelper.h"
#include "MathHelper.h"
using namespace DirectX;
using namespace DirectX::PackedVector;

#pragma region Effect

Effect::Effect( ID3D11Device* device, const char* vsFilename, const char* psFilename ) :
	mPSBlob { nullptr },
	mVSBlob { nullptr },
	mPixelShader { nullptr },
	mVertexShader { nullptr }
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	// Load cso files and create shaders
	HR( ShaderHelper::LoadCompiledShader( psFilename, &mPSBlob ) );
	HR( device->CreatePixelShader( mPSBlob->GetBufferPointer(), mPSBlob->GetBufferSize(), NULL, &mPixelShader ) );

	HR( ShaderHelper::LoadCompiledShader( vsFilename, &mVSBlob ) );
	HR( device->CreateVertexShader( mVSBlob->GetBufferPointer(), mVSBlob->GetBufferSize(), NULL, &mVertexShader ) );

}

Effect::~Effect()
{
	ReleaseCOM( mPSBlob );
	ReleaseCOM( mVSBlob );
	ReleaseCOM( mPixelShader );
	ReleaseCOM( mVertexShader );
}

void Effect::SetVertexShader( ID3D11DeviceContext* deviceContext )
{
	deviceContext->VSSetShader( mVertexShader, NULL, 0 );
}

void Effect::SetPixelShader( ID3D11DeviceContext* deviceContext )
{
	deviceContext->PSSetShader( mPixelShader, NULL, 0 );
}

#pragma endregion

#pragma region BasicEffect

BasicEffect::BasicEffect( ID3D11Device* device, const char* vsFilename, const char* psFilename ) :
	Effect( device, vsFilename, psFilename )
{
	mCBPerFrame.Initialize( device );
	mCBPerObject.Initialize( device );
}

BasicEffect::~BasicEffect()
{
}

void BasicEffect::SetPerFrameData( ID3D11DeviceContext* deviceContext, DirectionalLight& dirLight, XMFLOAT3& eyePosW )
{
	SetDirectionalLights( dirLight );
	SetEyePosWorld( eyePosW );

	ApplyPerFrameChanges( deviceContext );
}

void BasicEffect::SetPerObjectData( ID3D11DeviceContext* deviceContext, DirectX::XMFLOAT4X4& world, DirectX::XMFLOAT4X4& view, DirectX::XMFLOAT4X4& proj, Material& material )
{
	SetWorldMatrix( world );
	SetWorldInvTransposeMatrix( world );
	SetWorldViewProjMatrix( world, view, proj );
	SetMaterial( material );

	ApplyPerObjectChanges( deviceContext );
}

void BasicEffect::Render( ID3D11DeviceContext* deviceContext )
{
	// Set vertex and pixel shaders
	SetVertexShader( deviceContext );
	SetPixelShader( deviceContext );

	auto perFrameBuffer = GetPerFrameBuffer();
	auto perObjectBuffer = GetPerObjectBuffer();

	deviceContext->VSSetConstantBuffers( 0, 1, &perObjectBuffer );
	deviceContext->PSSetConstantBuffers( 0, 1, &perObjectBuffer );
	deviceContext->PSSetConstantBuffers( 1, 1, &perFrameBuffer );
}

void BasicEffect::SetWorldMatrix( DirectX::XMFLOAT4X4&  world )
{
	XMStoreFloat4x4( &mConstantsPerObject.m_World, XMMatrixTranspose( XMLoadFloat4x4( &world ) ) );
}

void BasicEffect::SetWorldInvTransposeMatrix( DirectX::XMFLOAT4X4&  world )
{
	XMMATRIX worldMat = XMLoadFloat4x4( &world );
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose( worldMat );
	XMStoreFloat4x4( &mConstantsPerObject.m_WorldInvTranspose, XMMatrixTranspose( worldInvTranspose ) );
}

void BasicEffect::SetWorldViewProjMatrix( DirectX::XMFLOAT4X4& world, DirectX::XMFLOAT4X4& view, DirectX::XMFLOAT4X4& proj )
{
	XMMATRIX worldMat = XMLoadFloat4x4( &world );
	XMMATRIX viewMat = XMLoadFloat4x4( &view );
	XMMATRIX projMat = XMLoadFloat4x4( &proj );
	XMMATRIX worldViewProj = worldMat*viewMat*projMat;

	XMStoreFloat4x4( &mConstantsPerObject.m_WorldViewProj, XMMatrixTranspose( worldViewProj ) );
}

void BasicEffect::SetMaterial( Material& material )
{
	mConstantsPerObject.mMaterial = material;
}

void BasicEffect::SetDirectionalLights( DirectionalLight& lights )
{
	mConstantsPerFrame.mDirLights = lights;
}

void BasicEffect::SetEyePosWorld( XMFLOAT3& eyePos )
{
	mConstantsPerFrame.mEyePosW = eyePos;
}

void BasicEffect::ApplyPerObjectChanges( ID3D11DeviceContext* deviceContext )
{
	mCBPerObject.Data = mConstantsPerObject;
	mCBPerObject.ApplyChanges( deviceContext );
}

void BasicEffect::ApplyPerFrameChanges( ID3D11DeviceContext* deviceContext )
{
	mCBPerFrame.Data = mConstantsPerFrame;
	mCBPerFrame.ApplyChanges( deviceContext );
}

ID3D11Buffer* BasicEffect::GetPerFrameBuffer() const
{
	return mCBPerFrame.Buffer();
}

ID3D11Buffer* BasicEffect::GetPerObjectBuffer() const
{
	return mCBPerObject.Buffer();
}

#pragma endregion

BasicEffect* Effects::BasicFX = nullptr;

void Effects::InitAll( ID3D11Device* device )
{
	BasicFX = new BasicEffect( device, "LightingVertexShader.cso", "LightingPixelShader.cso" );
}

void Effects::DestroyAll()
{
	SafeDelete( BasicFX );
}



#pragma endregion
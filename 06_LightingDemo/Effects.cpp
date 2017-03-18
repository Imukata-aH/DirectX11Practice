#include "Effects.h"
#include "ShaderHelper.h"

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
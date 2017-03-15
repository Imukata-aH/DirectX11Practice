﻿#include "Effects.h"
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

#pragma endregion

#pragma region BasicEffect

BasicEffect::BasicEffect( ID3D11Device* device, const char* vsFilename, const char* psFilename ) :
	Effect( device, vsFilename, psFilename )
{
}

BasicEffect::~BasicEffect()
{
}

void BasicEffect::ApplyChanges( ID3D11DeviceContext* deviceContext )
{
	mCBPerFrame.Data = mConstantsPerFrame;
	mCBPerFrame.ApplyChanges( deviceContext );

	mCBPerObject.Data = mConstantsPerObject;
	mCBPerObject.ApplyChanges( deviceContext );
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
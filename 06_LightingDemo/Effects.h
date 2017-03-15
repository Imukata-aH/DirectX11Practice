#pragma once

#include "d3dUtil.h"
#include "ConstantBuffer.h"

#pragma region Effect

class Effect
{
public:
	Effect( ID3D11Device* device, const char* vsFilename, const char* psFilename );
	virtual ~Effect();

private:
	Effect( const Effect& other ) {};
	Effect& operator=( const Effect& other ) {};

protected:
	ID3DBlob* mPSBlob;
	ID3DBlob* mVSBlob;
	ID3D11PixelShader* mPixelShader;
	ID3D11VertexShader* mVertexShader;
};

#pragma endregion

#pragma region BasicEffect

class BasicEffect : public Effect
{
public:
	BasicEffect( ID3D11Device* device, const char* vsFilename, const char* psFilename );
	~BasicEffect();

	void SetWorldMatrix( DirectX::XMFLOAT4X4  world ) { mConstantsPerObject.m_World = world; }
	void SetWorldInvTransposeMatrix( DirectX::XMFLOAT4X4  wordInvT ) { mConstantsPerObject.m_WorldInvTranspose = wordInvT; }
	void SetWorldViewProjMatrix( DirectX::XMFLOAT4X4  wvp ) { mConstantsPerObject.m_WorldViewProj = wvp; }
	void SetMaterial( Material material ) { mConstantsPerObject.mMaterial = material; }
	
	void SetDirectionalLights( DirectionalLight lights ) { mConstantsPerFrame.mDirLights = lights; }
	void SetEyePosWorld( XMFLOAT3 eyePos ) { mConstantsPerFrame.mEyePosW = eyePos; }

	void ApplyChanges( ID3D11DeviceContext* deviceContext );

private:
	struct ConstantsPerObjectBasic
	{
		DirectX::XMFLOAT4X4 m_World;
		DirectX::XMFLOAT4X4 m_WorldInvTranspose;
		DirectX::XMFLOAT4X4 m_WorldViewProj;
		Material mMaterial;
	};

	struct ConstantsPerFrameBasic
	{
		DirectionalLight mDirLights;
		XMFLOAT3 mEyePosW;
	};

	// Constant Buffer
	ConstantBuffer<ConstantsPerFrameBasic> mCBPerFrame;
	ConstantBuffer<ConstantsPerObjectBasic> mCBPerObject;

	ConstantsPerFrameBasic mConstantsPerFrame;
	ConstantsPerObjectBasic mConstantsPerObject;
};

#pragma endregion

#pragma region Effects

class Effects
{
public:
	static void InitAll( ID3D11Device* device );
	static void DestroyAll();

	static BasicEffect* BasicFX;
};

#pragma endregion
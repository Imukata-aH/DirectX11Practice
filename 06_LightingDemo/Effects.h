#pragma once

#include "d3dUtil.h"
#include "ConstantBuffer.h"

#pragma region Effect

class Effect
{
public:
	Effect( ID3D11Device* device, const char* vsFilename, const char* psFilename );
	virtual ~Effect();

	void SetVertexShader( ID3D11DeviceContext* deviceContext );
	void SetPixelShader( ID3D11DeviceContext* deviceContext );

	ID3DBlob* GetPSBlob() { return mPSBlob; }
	ID3DBlob* GetVSBlob() { return mVSBlob; }

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

	void SetPerFrameData( ID3D11DeviceContext* deviceContext, std::vector<DirectionalLight>& lights, XMFLOAT3& eyePosW );
	void SetPerObjectData( ID3D11DeviceContext* deviceContext, DirectX::XMFLOAT4X4& world, DirectX::XMFLOAT4X4& view, DirectX::XMFLOAT4X4& proj, Material& material );
	void Render( ID3D11DeviceContext* deviceContext, ID3D11InputLayout* inputLayout );

private:
	void SetWorldMatrix( DirectX::XMFLOAT4X4&  world );
	void SetWorldInvTransposeMatrix( DirectX::XMFLOAT4X4&  world);
	void SetWorldViewProjMatrix( DirectX::XMFLOAT4X4& world, DirectX::XMFLOAT4X4& view, DirectX::XMFLOAT4X4& proj );
	void SetMaterial( Material& material );
	
	void SetDirectionalLights( std::vector<DirectionalLight>& lights );
	void SetEyePosWorld( XMFLOAT3& eyePos );

	void ApplyPerObjectChanges( ID3D11DeviceContext* deviceContext );
	void ApplyPerFrameChanges( ID3D11DeviceContext* deviceContext );

	ID3D11Buffer* GetPerFrameBuffer() const;
	ID3D11Buffer* GetPerObjectBuffer() const;

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
		DirectionalLight mDirLights[3];
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
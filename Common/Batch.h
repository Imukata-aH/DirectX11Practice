#pragma once

#include <vector>

#include "d3dUtil.h"
#include "ConstantBuffer.h"

struct ConstantsPerObject
{
	DirectX::XMFLOAT4X4 m_WorldViewProj;
};

class Batch
{
public:
	Batch( ID3D11Device** device, ID3D11DeviceContext** deviceContext, ID3D11Buffer** vertexBuffer, ID3D11Buffer** indexBuffer, UINT indexCount, UINT stride, UINT offset );
	~Batch();

	void Release();

	void Draw( const XMMATRIX& wvmMatrix, ConstantBuffer<ConstantsPerObject>* constantBuffer );

private:
	ID3D11Device** m_device;
	ID3D11DeviceContext** m_deviceContext;

	ID3D11Buffer** m_vb;
	ID3D11Buffer** m_ib;

	UINT m_indexCount;
	UINT m_stride;
	UINT m_offset;
};
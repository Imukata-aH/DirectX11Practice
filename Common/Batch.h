#pragma once

#include <vector>

#include "d3dUtil.h"
#include "ConstantBuffer.h"

struct Vertex
{
	XMFLOAT3 Position;
	XMFLOAT4 Color;
};

struct ConstantsPerObject
{
	DirectX::XMFLOAT4X4 m_WorldViewProj;
};

class Batch
{
public:
	Batch( ID3D11Device** device, ID3D11DeviceContext** deviceContext, std::vector<Vertex>* vertices, std::vector<UINT>* indices);
	~Batch();

	void Release();

	void Draw( const XMMATRIX& wvmMatrix );

private:
	ID3D11Device** m_device;
	ID3D11DeviceContext** m_deviceContext;

	ID3D11Buffer* m_vb;
	ID3D11Buffer* m_ib;

	UINT m_indexCount;
	// TODO: ConstantBuffer のポインタを保持してインスタンス自体は使いまわすべき
	ConstantBuffer<ConstantsPerObject> m_constantBuffer;
};
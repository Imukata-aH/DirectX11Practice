﻿#include "Batch.h"

Batch::Batch( ID3D11Device** device, ID3D11DeviceContext** deviceContext, std::vector<Vertex>* vertices, std::vector<UINT>* indices ) :
	m_device(device),
	m_deviceContext(deviceContext),
	m_vb( 0 ),
	m_ib( 0),
	m_indexCount(0)
{
	m_constantBuffer.Initialize( *m_device );

	// 頂点バッファの構築

	D3D11_BUFFER_DESC vbd;

	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof( Vertex ) * vertices->size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &( ( *vertices )[0] );

	HR( ( *device )->CreateBuffer( &vbd, &vinitData, &m_vb ) );


	// インデックスバッファの構築

	m_indexCount = indices->size();

	D3D11_BUFFER_DESC ibd;

	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof( UINT ) * indices->size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &( ( *indices )[0] );

	HR( ( *device )->CreateBuffer( &ibd, &iinitData, &m_ib ) );
}

Batch::~Batch()
{
}

void Batch::Release()
{
	ReleaseCOM( m_vb );
	ReleaseCOM( m_ib );
}

void Batch::Draw( const XMMATRIX& wvmMatrix )
{
	// Constant Buffer の更新

	ConstantsPerObject constants;

	XMStoreFloat4x4( &constants.m_WorldViewProj, wvmMatrix );
	m_constantBuffer.Data = constants;
	m_constantBuffer.ApplyChanges( *m_deviceContext );

	auto buffer = m_constantBuffer.Buffer();
	(*m_deviceContext)->VSSetConstantBuffers( 0, 1, &buffer );

	// 頂点バッファのセット
	UINT stride = sizeof( Vertex );
	UINT offset = 0;
	( *m_deviceContext )->IASetVertexBuffers( 0, 1, &m_vb, &stride, &offset );

	// インデックスバッファのセット
	( *m_deviceContext )->IASetIndexBuffer( m_ib, DXGI_FORMAT_R32_UINT, 0 );

	// 描画
	( *m_deviceContext )->DrawIndexed( m_indexCount, 0, 0 );
}
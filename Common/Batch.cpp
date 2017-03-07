#include "Batch.h"
#include <DirectXColors.h>
using namespace DirectX;

Batch::Batch( ID3D11Device** device, ID3D11DeviceContext** deviceContext, ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer, UINT indexCount, UINT stride, UINT offset) :
	m_device( device ),
	m_deviceContext( deviceContext ),
	m_vb( vertexBuffer ),
	m_ib( indexBuffer ),
	m_indexCount( indexCount ),
	m_stride( stride ),
	m_offset( offset )
{

}

Batch::~Batch()
{
}

void Batch::Release()
{
	ReleaseCOM( ( m_vb ) );
	ReleaseCOM( ( m_ib ) );
}

void Batch::Draw( const XMMATRIX& wvmMatrix, ConstantBuffer<ConstantsPerObject>* constantBuffer )
{
	//// Constant Buffer の更新

	ConstantsPerObject constants;

	XMStoreFloat4x4( &constants.m_WorldViewProj, wvmMatrix );
	constantBuffer->Data = constants;
	constantBuffer->ApplyChanges( *m_deviceContext );

	auto buffer = constantBuffer->Buffer();
	( *m_deviceContext )->VSSetConstantBuffers( 0, 1, &buffer );

	// 頂点バッファのセット
	( *m_deviceContext )->IASetVertexBuffers( 0, 1, &m_vb, &m_stride, &m_offset );

	// インデックスバッファのセット
	( *m_deviceContext )->IASetIndexBuffer( m_ib, DXGI_FORMAT_R32_UINT, 0 );

	// 描画
	( *m_deviceContext )->DrawIndexed( m_indexCount, 0, 0 );
}
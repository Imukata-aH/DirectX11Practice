#pragma once

#include "d3dUtil.h"

template <typename T>
class BufferHelper
{
public:
	static bool CreateVertexBuffer( ID3D11Device** device, const std::vector<T>& vertices, ID3D11Buffer** vertexBuffer )
	{
		if ( vertices.size() == 0 )
		{

			return false;
		}

		D3D11_BUFFER_DESC vbd;

		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof( T ) * vertices.size();
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vinitData;
		vinitData.pSysMem = &( vertices[0] );

		HR( ( *device )->CreateBuffer( &vbd, &vinitData, vertexBuffer ) );

		return true;
	}

	static bool CreateIndexBuffer( ID3D11Device** device, const std::vector<T>& indices, ID3D11Buffer** indexBuffer )
	{
		if ( indices.size() == 0 )
		{

			return false;
		}

		D3D11_BUFFER_DESC ibd;

		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof( T ) * indices.size();
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA iinitData;
		iinitData.pSysMem = &( indices[0] );

		HR( ( *device )->CreateBuffer( &ibd, &iinitData, indexBuffer ) );

		return true;
	}
};
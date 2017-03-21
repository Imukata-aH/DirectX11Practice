#pragma once

#include "d3dUtil.h"

namespace Vertex
{
	struct PosNormal
	{
		XMFLOAT3 Position;
		XMFLOAT3 Normal;
	};
}

class InputLayoutDesc
{
public:
	static const D3D11_INPUT_ELEMENT_DESC PosNormal[2];
};

class InputLayouts
{
public:
	static void InitAll( ID3D11Device* device, ID3DBlob* vsBlob );
	static void DestroyAll();

	static ID3D11InputLayout* PosNormal;
};
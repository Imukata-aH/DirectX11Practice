#include "Vertex.h"

#pragma region InputLayoutDesc

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::PosNormal[2] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

#pragma endregion

#pragma region InputLayouts

ID3D11InputLayout* InputLayouts::PosNormal = 0;

void InputLayouts::InitAll( ID3D11Device* device, ID3DBlob* vsBlob )
{
	// PosNormal

	HR( device->CreateInputLayout( InputLayoutDesc::PosNormal, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &PosNormal ) );
}

void InputLayouts::DestroyAll()
{
	ReleaseCOM( PosNormal );
}

#pragma endregion
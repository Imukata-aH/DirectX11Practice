#pragma once

#include "d3dUtil.h"
#include "ConstantBuffer.h"
class Batch;
struct ConstantsPerObject;

class Model
{
public:
	Model( Batch* batch );
	~Model();

	void Release();

	void Draw( const XMMATRIX& viewProjectionMatrix, ConstantBuffer<ConstantsPerObject>* constantBuffer );
	void SetPosition( const XMFLOAT3& position );
	void SetScale( const XMFLOAT3& scale );
	void SetRotation( const XMFLOAT3& angle );

private:
	Batch* m_batch;

	XMFLOAT3 m_position;
	XMFLOAT3 m_scale;
	XMFLOAT3 m_rotation;
};
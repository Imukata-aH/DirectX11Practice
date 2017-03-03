#include "Model.h"
#include "Batch.h"

Model::Model( Batch* batch ) :
	m_batch( batch ),
	m_position( XMFLOAT3( 0.0f, 0.0f, 0.0f ) ),
	m_scale( XMFLOAT3( 1.0f, 1.0f, 1.0f ) ),
	m_rotation( XMFLOAT3( 0.0f, 0.0f, 0.0f ) )
{

}

Model::~Model()
{
}

void Model::Release()
{
	m_batch->Release();
}

void Model::Draw( const XMMATRIX& viewProjectionMatrix )
{
	// World matrix の構築

	/*XMVECTOR scaleVec = XMLoadFloat3( &m_scale );
	XMMATRIX scale = XMMatrixScalingFromVector( scaleVec );

	XMVECTOR rotationVec = XMLoadFloat3( &m_rotation );
	XMMATRIX rotation = XMMatrixRotationRollPitchYawFromVector( rotationVec );

	XMVECTOR transitionVec = XMLoadFloat3( &m_position );
	XMMATRIX transition = XMMatrixTranslationFromVector( transitionVec );

	XMMATRIX world = scale * transition * rotation;*/
	XMMATRIX world = XMMatrixIdentity();

	// Transpose is needed from DirectxMath's spec.
	XMMATRIX worldViewProj = XMMatrixTranspose( world * viewProjectionMatrix );

	// 描画
	m_batch->Draw( worldViewProj );
}

void Model::SetPosition( const XMFLOAT3& position )
{
	m_position = position;
}

void Model::SetScale( const XMFLOAT3& scale )
{
	m_scale = scale;
}

void Model::SetRotation( const XMFLOAT3& angle )
{
	m_rotation = angle;
}
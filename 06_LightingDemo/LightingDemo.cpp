#include <string>

#include "d3dApp.h"
#include "d3dUtil.h"
#include "DirectXColors.h"
#include "BufferHelper.h"
#include "LightHelper.h"
#include "Vertex.h"
#include "Effects.h"

// Assimp
#include "Importer.hpp"
#include "scene.h"
#include "postprocess.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

#define MESH_FILE "suzanne.obj"

class LightingApp : public D3DApp
{
public:
	LightingApp( HINSTANCE hInstance );
	~LightingApp();

	bool Init();
	void OnResize();
	void UpdateScene( float dt );
	void DrawScene();

	void OnMouseDown( WPARAM btnState, int x, int y );
	void OnMouseUp( WPARAM btnState, int x, int y );
	void OnMouseMove( WPARAM btnState, int x, int y );

private:
	void BuildGeometryBuffers();
	void BuildRasterState();
	void BuildWireFrameRasterState();

	bool ImportMeshFromFile( const std::string & filename, ID3D11Buffer** vertexBuffer, ID3D11Buffer** indexBuffer, UINT* indexCount );

private:
	ID3D11RasterizerState* mRasterState;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	ID3D11Buffer* mMonkeyVB;
	ID3D11Buffer* mMonkeyIB;
	UINT mMonkeyIndexCount;
	XMFLOAT4X4 mMonkeyWorldMat;
	Material mMonkeyMaterial;
	
	std::vector<DirectionalLight> mDirLights = std::vector<DirectionalLight>(3);
	XMFLOAT3 mEyePosW;

	std::vector<bool> mEnableDirLights = std::vector<bool>(3);

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE prevInstance,
					PSTR cmdLine, int showCmd )
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	LightingApp theApp( hInstance );

	if ( !theApp.Init() )
		return 0;

	return theApp.Run();
}


LightingApp::LightingApp( HINSTANCE hInstance )
	: D3DApp( hInstance ),
	mTheta( 0.1f*MathHelper::Pi ), mPhi( 0.5f*MathHelper::Pi ), mRadius( 10.0f ), mEyePosW( 0.0f, 0.0f, 0.0f )
{
	mMainWndCaption = L"Lighting Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	for ( size_t i = 0; i < mEnableDirLights.size(); i++ )
	{
		mEnableDirLights[i] = true;
	}

	// Directional light.
	mDirLights[0].Ambient = XMFLOAT4( 0.2f, 0.2f, 0.2f, 1.0f );
	mDirLights[0].Diffuse = XMFLOAT4( 0.5f, 0.5f, 0.5f, 1.0f );
	mDirLights[0].Specular = XMFLOAT4( 0.5f, 0.5f, 0.5f, 1.0f );
	mDirLights[0].Direction = XMFLOAT3( 0.57735f, -0.57735f, 0.57735f );
	mDirLights[0].Pad = mEnableDirLights[0] ? 1.0f : 0.0f;

	mDirLights[1].Ambient = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );
	mDirLights[1].Diffuse = XMFLOAT4( 0.20f, 0.20f, 0.20f, 1.0f );
	mDirLights[1].Specular = XMFLOAT4( 0.25f, 0.25f, 0.25f, 1.0f );
	mDirLights[1].Direction = XMFLOAT3( -0.57735f, -0.57735f, 0.57735f );
	mDirLights[1].Pad = mEnableDirLights[1] ? 1.0f : 0.0f;

	mDirLights[2].Ambient = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );
	mDirLights[2].Diffuse = XMFLOAT4( 0.2f, 0.2f, 0.2f, 1.0f );
	mDirLights[2].Specular = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );
	mDirLights[2].Direction = XMFLOAT3( 0.0f, -0.707f, -0.707f );
	mDirLights[2].Pad = mEnableDirLights[2] ? 1.0f : 0.0f;

	// Material
	mMonkeyMaterial.Ambient = XMFLOAT4( 0.48f, 0.77f, 0.46f, 1.0f );
	mMonkeyMaterial.Diffuse = XMFLOAT4( 0.48f, 0.77f, 0.46f, 1.0f );
	mMonkeyMaterial.Specular = XMFLOAT4( 0.2f, 0.2f, 0.2f, 16.0f );

	// Wrold pos
	XMStoreFloat4x4( &mMonkeyWorldMat, XMMatrixIdentity() );
}

LightingApp::~LightingApp()
{
	InputLayouts::DestroyAll();
	Effects::DestroyAll();
	ReleaseCOM( md3dImmediateContext );
	ReleaseCOM( md3dDevice );
}

bool LightingApp::Init()
{
	if ( !D3DApp::Init() )
		return false;

	BuildGeometryBuffers();

	Effects::InitAll( md3dDevice );
	InputLayouts::InitAll( md3dDevice, Effects::BasicFX->GetVSBlob() );

	BuildRasterState();
	//BuildWireFrameRasterState();

	return true;
}

void LightingApp::OnResize()
{
	D3DApp::OnResize();

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH( 0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f );
	XMStoreFloat4x4( &mProj, P );
}

void LightingApp::UpdateScene( float dt )
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius*sinf( mPhi )*cosf( mTheta );
	float z = mRadius*sinf( mPhi )*sinf( mTheta );
	float y = mRadius*cosf( mPhi );

	mEyePosW = XMFLOAT3( x, y, z );

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet( x, y, z, 1.0f );
	//XMVECTOR pos = XMVectorSet( 1.0f, 0.0f, -5.0f, 1.0f );
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );

	XMMATRIX V = XMMatrixLookAtLH( pos, target, up );
	XMStoreFloat4x4( &mView, V );

	// Update lights strength with button push
	if ( GetAsyncKeyState( '0' ) & 0x8000 )
	{
		mEnableDirLights[0] = !mEnableDirLights[0];
		mDirLights[0].Pad = mEnableDirLights[0] ? 1.0f : 0.0f;
	}

	if ( GetAsyncKeyState( '1' ) & 0x8000 )
	{
		mEnableDirLights[1] = !mEnableDirLights[1];
		mDirLights[1].Pad = mEnableDirLights[1] ? 1.0f : 0.0f;
	}

	if ( GetAsyncKeyState( '2' ) & 0x8000 )
	{
		mEnableDirLights[2] = !mEnableDirLights[2];
		mDirLights[2].Pad = mEnableDirLights[2] ? 1.0f : 0.0f;
	}

	if ( GetAsyncKeyState( '3' ) & 0x8000 )
	{
		// All lights enabled
		for ( size_t i = 0; i < mDirLights.size(); i++ )
		{
			mEnableDirLights[i] = true;
			mDirLights[i].Pad = 1.0f;
		}
	}
}

void LightingApp::DrawScene()
{
	md3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	md3dImmediateContext->ClearRenderTargetView( mRenderTargetView, reinterpret_cast<const float*>( &Colors::LightSteelBlue ) );
	md3dImmediateContext->ClearDepthStencilView( mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );

	// Set per frame constants
	Effects::BasicFX->SetPerFrameData( md3dImmediateContext, mDirLights, mEyePosW );
	
	// Set raster state
	md3dImmediateContext->RSSetState( mRasterState );

	// Set up rendering effect
	Effects::BasicFX->SetPerObjectData( md3dImmediateContext, mMonkeyWorldMat, mView, mProj, mMonkeyMaterial );
	Effects::BasicFX->Render( md3dImmediateContext, InputLayouts::PosNormal );

	// 頂点バッファのセット
	UINT stride = sizeof( Vertex::PosNormal );
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers( 0, 1, &mMonkeyVB, &stride, &offset );

	// インデックスバッファのセット
	md3dImmediateContext->IASetIndexBuffer( mMonkeyIB, DXGI_FORMAT_R32_UINT, 0 );

	// 描画
	md3dImmediateContext->DrawIndexed( mMonkeyIndexCount, 0, 0 );

	HR( mSwapChain->Present( 0, 0 ) );
}

void LightingApp::OnMouseDown( WPARAM btnState, int x, int y )
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture( mhMainWnd );
}

void LightingApp::OnMouseUp( WPARAM btnState, int x, int y )
{
	ReleaseCapture();
}

void LightingApp::OnMouseMove( WPARAM btnState, int x, int y )
{
	if ( ( btnState & MK_LBUTTON ) != 0 )
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians( 0.25f*static_cast<float>( x - mLastMousePos.x ) );
		float dy = XMConvertToRadians( 0.25f*static_cast<float>( y - mLastMousePos.y ) );

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi += dy;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp( mPhi, 0.1f, MathHelper::Pi - 0.1f );
	}
	else if ( ( btnState & MK_RBUTTON ) != 0 )
	{
		// Make each pixel correspond to 0.005 unit in the scene.
		float dx = 0.1f*static_cast<float>( x - mLastMousePos.x );
		float dy = 0.1f*static_cast<float>( y - mLastMousePos.y );

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp( mRadius, 3.0f, 250.0f );
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void LightingApp::BuildGeometryBuffers()
{
	bool result = ImportMeshFromFile( MESH_FILE, &mMonkeyVB, &mMonkeyIB, &mMonkeyIndexCount );
	if ( !result )
	{
		OutputDebugString( L"Reading mesh file failed.\n" );
	}
}

void LightingApp::BuildRasterState()
{
	D3D11_RASTERIZER_DESC rs;
	memset( &rs, 0, sizeof( rs ) );
	rs.FillMode = D3D11_FILL_SOLID;
	rs.CullMode = D3D11_CULL_BACK;
	rs.AntialiasedLineEnable = rs.DepthClipEnable = true;
	mRasterState = NULL;
	HR( md3dDevice->CreateRasterizerState( &rs, &mRasterState ) );
}

void LightingApp::BuildWireFrameRasterState()
{
	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory( &wireframeDesc, sizeof( D3D11_RASTERIZER_DESC ) );
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_NONE;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;
	mRasterState = NULL;
	HR( md3dDevice->CreateRasterizerState( &wireframeDesc, &mRasterState ) );
}

bool LightingApp::ImportMeshFromFile( const std::string & filename, ID3D11Buffer** vertexBuffer, ID3D11Buffer** indexBuffer, UINT* indexCount )
{
	wchar_t msg[256];

	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile( filename, aiProcess_Triangulate );
	if ( !scene )
	{
		fprintf( stderr, "ERROR: reading mesh %s\n", filename );
		return false;
	}

	swprintf_s( msg, 256, L"  %i meshes\n", scene->mNumMeshes );
	OutputDebugString( msg );

	const aiMesh* mesh = scene->mMeshes[0];
	swprintf_s( msg, 256, L"  %i vertices in mesh[0]\n", mesh->mNumVertices );
	OutputDebugString( msg );

	std::vector<Vertex::PosNormal> vertices = std::vector<Vertex::PosNormal>( static_cast<size_t>( mesh->mNumVertices ) );

	if ( mesh->HasPositions() )
	{
		for ( int i = 0; i < mesh->mNumVertices; i++ )
		{
			const aiVector3D* pos = &( mesh->mVertices[i] );
			vertices[i].Position = XMFLOAT3( pos->x, pos->y, pos->z );
		}
	}

	if ( mesh->HasNormals() )
	{
		for ( int i = 0; i < mesh->mNumVertices; i++ )
		{
			const aiVector3D* normal = &( mesh->mNormals[i] );
			vertices[i].Normal = XMFLOAT3( normal->x, normal->y, normal->z );
		}
	}

	std::vector<UINT> indices = std::vector<UINT>( mesh->mNumFaces * 3 );
	for ( int i = 0; i < mesh->mNumFaces; i++ )
	{
		const aiFace& face = mesh->mFaces[i];
		assert( face.mNumIndices == 3 );
		indices[( i * 3 ) + 0] = face.mIndices[0];
		indices[( i * 3 ) + 1] = face.mIndices[1];
		indices[( i * 3 ) + 2] = face.mIndices[2];
	}
	*indexCount = static_cast<UINT>( indices.size() );

	swprintf_s( msg, 256, L"Complete reading mesh %s\n", filename );

	BufferHelper<Vertex::PosNormal>::CreateVertexBuffer( &md3dDevice, vertices, vertexBuffer );
	BufferHelper<UINT>::CreateIndexBuffer( &md3dDevice, indices, indexBuffer );

	return true;
}

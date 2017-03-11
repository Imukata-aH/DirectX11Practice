#include <string>

#include "d3dApp.h"
#include "d3dUtil.h"
#include "MathHelper.h"
#include "DirectXColors.h"
#include "ConstantBuffer.h"
#include "ShaderHelper.h"
#include "GeometryGenerator.h"
#include "Batch.h"
#include "Model.h"
#include "BufferHelper.h"
#include "LightHelper.h"
#include "cbPerFrame.h"

// Assimp
#include "Importer.hpp"
#include "scene.h"
#include "postprocess.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

#define MESH_FILE "suzanne.obj"

struct Vertex
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
};


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
	void BuildFX();
	void BuildVertexLayout();
	void BuildRasterState();
	void BuildWireFrameRasterState();

	bool ImportMeshFromFile( const std::string & filename, std::vector<Vertex>** vertices, std::vector<UINT>** indices );

private:
	ConstantBuffer<ConstantsPerObject> mObjectConstantBuffer;
	ConstantBuffer<ConstantsPerFrame> mFrameConstantBuffer;
	ID3DBlob* mPSBlob;
	ID3DBlob* mVSBlob;
	ID3D11PixelShader* mPixelShader;
	ID3D11VertexShader* mVertexShader;
	ID3D11InputLayout* mInputLayout;
	ID3D11RasterizerState* mRasterState;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	Model* m_importedMeshModel;
	
	DirectionalLight mDirLight;
	PointLight mPointLight;
	SpotLight mSpotLight;
	XMFLOAT3 mEyePosW;

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
	: D3DApp( hInstance ), mInputLayout( 0 ),
	mTheta( 0.1f*MathHelper::Pi ), mPhi( 0.5f*MathHelper::Pi ), mRadius( 10.0f ), mEyePosW( 0.0f, 0.0f, 0.0f )
{
	mMainWndCaption = L"Lighting Demo";

	// Directional light.
	mDirLight.Ambient = XMFLOAT4( 0.1f, 0.1f, 0.1f, 1.0f );
	mDirLight.Diffuse = XMFLOAT4( 0.5f, 0.5f, 0.5f, 1.0f );
	mDirLight.Specular = XMFLOAT4( 0.5f, 0.5f, 0.5f, 1.0f );
	mDirLight.Direction = XMFLOAT3( 0.57735f, -0.57735f, 0.57735f );

	mPointLight.Ambient = XMFLOAT4( 0.1f, 0.1f, 0.1f, 1.0f );
	mPointLight.Diffuse = XMFLOAT4( 0.7f, 0.7f, 0.7f, 1.0f );
	mPointLight.Specular = XMFLOAT4( 0.7f, 0.7f, 0.7f, 1.0f );
	mPointLight.Att = XMFLOAT3( 0.0f, 0.1f, 0.0f );
	mPointLight.Range = 25.0f;

	mSpotLight.Ambient = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );
	mSpotLight.Diffuse = XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f );
	mSpotLight.Specular = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	mSpotLight.Att = XMFLOAT3( 1.0f, 0.0f, 0.0f );
	mSpotLight.Spot = 96.0f;
	mSpotLight.Range = 10000.0f;

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;
}

LightingApp::~LightingApp()
{
	m_importedMeshModel->Release();
	delete m_importedMeshModel;
	m_importedMeshModel = 0;

	ReleaseCOM( mInputLayout );
	ReleaseCOM( mPSBlob );
	ReleaseCOM( mVSBlob );
	ReleaseCOM( md3dImmediateContext );
	ReleaseCOM( md3dDevice );
}

bool LightingApp::Init()
{
	if ( !D3DApp::Init() )
		return false;

	BuildGeometryBuffers();
	BuildFX();
	BuildVertexLayout();

	BuildRasterState();
	//BuildWireFrameRasterState();

	mObjectConstantBuffer.Initialize( md3dDevice );
	mFrameConstantBuffer.Initialize( md3dDevice );

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

	// circulate point light
	mPointLight.Position.x = 10.0f*cosf( 0.2f*mTimer.TotalTime() );
	mPointLight.Position.z = 10.0f*sinf( 0.2f*mTimer.TotalTime() );
	mPointLight.Position.y = 0.0f;

	// The spotlight takes on the camera position and is aimed in the
	// same direction the camera is looking.  In this way, it looks
	// like we are holding a flashlight.
	mSpotLight.Position = mEyePosW;
	XMStoreFloat3( &mSpotLight.Direction, XMVector3Normalize( target - pos ) );
}

void LightingApp::DrawScene()
{
	md3dImmediateContext->IASetInputLayout( mInputLayout );
	md3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	md3dImmediateContext->ClearRenderTargetView( mRenderTargetView, reinterpret_cast<const float*>( &Colors::LightSteelBlue ) );
	md3dImmediateContext->ClearDepthStencilView( mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );

	// Apply lights data
	ConstantsPerFrame cbPerFrame;
	cbPerFrame.mDirLight = mDirLight;
	cbPerFrame.mPointLight = mPointLight;
	cbPerFrame.mSpotLight = mSpotLight;
	cbPerFrame.mEyePosW = mEyePosW;
	mFrameConstantBuffer.Data = cbPerFrame;
	mFrameConstantBuffer.ApplyChanges( md3dImmediateContext );

	auto buffer = mFrameConstantBuffer.Buffer();
	md3dImmediateContext->PSSetConstantBuffers( 1, 1, &buffer );

	// Set vertex and pixel shaders
	md3dImmediateContext->PSSetShader( mPixelShader, NULL, 0 );
	md3dImmediateContext->VSSetShader( mVertexShader, NULL, 0 );

	// Set raster state
	md3dImmediateContext->RSSetState( mRasterState );

	XMMATRIX view = XMLoadFloat4x4( &mView );
	XMMATRIX proj = XMLoadFloat4x4( &mProj );
	XMMATRIX viewProj = view*proj;

	m_importedMeshModel->Draw( viewProj, &mObjectConstantBuffer );

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
	std::vector<Vertex>* vertices = nullptr;
	std::vector<UINT>* indices = nullptr;
	bool result = ImportMeshFromFile( MESH_FILE, &vertices, &indices );
	if ( !result )
	{
		OutputDebugString( L"Reading mesh file failed.\n" );
	}

	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11Buffer* indexBuffer = nullptr;

	BufferHelper<Vertex>::CreateVertexBuffer( &md3dDevice, *vertices, &vertexBuffer );
	BufferHelper<UINT>::CreateIndexBuffer( &md3dDevice, *indices, &indexBuffer );

	Material material;
	material.Ambient = XMFLOAT4( 0.48f, 0.77f, 0.46f, 1.0f );
	material.Diffuse = XMFLOAT4( 0.48f, 0.77f, 0.46f, 1.0f );
	material.Specular = XMFLOAT4( 0.2f, 0.2f, 0.2f, 16.0f );

	Batch* importexMeshBatch = new Batch( &md3dDevice, &md3dImmediateContext, vertexBuffer, indexBuffer, indices->size(), sizeof( Vertex ), 0, material );
	m_importedMeshModel = new Model( importexMeshBatch );

	delete vertices;
	vertices = 0;

	delete indices;
	indices = 0;
}

void LightingApp::BuildFX()
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	// Load cso files and create shaders
	HR( ShaderHelper::LoadCompiledShader( "LightingPixelShader.cso", &mPSBlob ) );
	HR( md3dDevice->CreatePixelShader( mPSBlob->GetBufferPointer(), mPSBlob->GetBufferSize(), NULL, &mPixelShader ) );

	HR( ShaderHelper::LoadCompiledShader( "LightingVertexShader.cso", &mVSBlob ) );
	HR( md3dDevice->CreateVertexShader( mVSBlob->GetBufferPointer(), mVSBlob->GetBufferSize(), NULL, &mVertexShader ) );
}

void LightingApp::BuildVertexLayout()
{
	// Create the vertex input layout.
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	HR( md3dDevice->CreateInputLayout( vertexDesc, 2, mVSBlob->GetBufferPointer(), mVSBlob->GetBufferSize(), &mInputLayout ) );
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

bool LightingApp::ImportMeshFromFile( const std::string & filename, std::vector<Vertex>** vertices, std::vector<UINT>** indices )
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

	*vertices = new std::vector<Vertex>( static_cast<size_t>( mesh->mNumVertices ) );

	if ( mesh->HasPositions() )
	{
		for ( int i = 0; i < mesh->mNumVertices; i++ )
		{
			const aiVector3D* pos = &( mesh->mVertices[i] );
			( **vertices )[i].Position = XMFLOAT3( pos->x, pos->y, pos->z );
		}
	}

	if ( mesh->HasNormals() )
	{
		for ( int i = 0; i < mesh->mNumVertices; i++ )
		{
			const aiVector3D* normal = &( mesh->mNormals[i] );
			( **vertices )[i].Normal = XMFLOAT3( normal->x, normal->y, normal->z );
		}
	}

	*indices = new std::vector<UINT>( mesh->mNumFaces * 3 );
	for ( int i = 0; i < mesh->mNumFaces; i++ )
	{
		const aiFace& face = mesh->mFaces[i];
		assert( face.mNumIndices == 3 );
		( **indices )[( i * 3 ) + 0] = face.mIndices[0];
		( **indices )[( i * 3 ) + 1] = face.mIndices[1];
		( **indices )[( i * 3 ) + 2] = face.mIndices[2];
	}

	swprintf_s( msg, 256, L"Complete reading mesh %s\n", filename );

	return true;
}

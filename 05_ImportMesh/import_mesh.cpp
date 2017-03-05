#include "d3dApp.h"
#include "d3dUtil.h"
#include "MathHelper.h"
#include "DirectXColors.h"
#include "ConstantBuffer.h"
#include "ShaderHelper.h"
#include "GeometryGenerator.h"
#include "Batch.h"
#include "Model.h"
using namespace DirectX;
using namespace DirectX::PackedVector;

class ShapesApp : public D3DApp
{
public:
	ShapesApp( HINSTANCE hInstance );
	~ShapesApp();

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

	float GetHeight( float x, float z )const;

private:
	ConstantBuffer<ConstantsPerObject> mObjectConstantBuffer;
	ID3DBlob* mPSBlob;
	ID3DBlob* mVSBlob;
	ID3D11PixelShader* mPixelShader;
	ID3D11VertexShader* mVertexShader;
	ID3D11InputLayout* mInputLayout;
	ID3D11RasterizerState* mRasterState;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	Model* m_boxModel;
	Model* m_gridModel;
	Model* m_cylinderModels[10];

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

	ShapesApp theApp( hInstance );

	if ( !theApp.Init() )
		return 0;

	return theApp.Run();
}


ShapesApp::ShapesApp( HINSTANCE hInstance )
	: D3DApp( hInstance ), mInputLayout( 0 ),
	mTheta( 0.8f*MathHelper::Pi ), mPhi( 0.1f*MathHelper::Pi ), mRadius( 10.0f )
{
	mMainWndCaption = L"Shapes Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;
}

ShapesApp::~ShapesApp()
{
	m_boxModel->Release();
	delete m_boxModel;
	m_boxModel = 0;

	m_gridModel->Release();
	delete m_gridModel;
	m_gridModel = 0;

	for ( int i = 0; i < 10; i++ )
	{
		m_cylinderModels[i]->Release();
		delete m_cylinderModels[i];
		m_cylinderModels[i] = 0;
	}

	ReleaseCOM( mInputLayout );
	ReleaseCOM( mPSBlob );
	ReleaseCOM( mVSBlob );
	ReleaseCOM( md3dImmediateContext );
	ReleaseCOM( md3dDevice );
}

bool ShapesApp::Init()
{
	if ( !D3DApp::Init() )
		return false;

	BuildGeometryBuffers();
	BuildFX();
	BuildVertexLayout();
	//BuildRasterState();
	BuildWireFrameRasterState();
	mObjectConstantBuffer.Initialize( md3dDevice );

	return true;
}

void ShapesApp::OnResize()
{
	D3DApp::OnResize();

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH( 0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f );
	XMStoreFloat4x4( &mProj, P );
}

void ShapesApp::UpdateScene( float dt )
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius*sinf( mPhi )*cosf( mTheta );
	float z = mRadius*sinf( mPhi )*sinf( mTheta );
	float y = mRadius*cosf( mPhi );

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet( x, y, z, 1.0f );
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );

	XMMATRIX V = XMMatrixLookAtLH( pos, target, up );
	XMStoreFloat4x4( &mView, V );
}

void ShapesApp::DrawScene()
{
	md3dImmediateContext->IASetInputLayout( mInputLayout );
	md3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	md3dImmediateContext->ClearRenderTargetView( mRenderTargetView, reinterpret_cast<const float*>( &Colors::LightSteelBlue ) );
	md3dImmediateContext->ClearDepthStencilView( mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );

	// Set vertex and pixel shaders
	md3dImmediateContext->PSSetShader( mPixelShader, NULL, 0 );
	md3dImmediateContext->VSSetShader( mVertexShader, NULL, 0 );

	// Set raster state
	md3dImmediateContext->RSSetState( mRasterState );

	XMMATRIX view = XMLoadFloat4x4( &mView );
	XMMATRIX proj = XMLoadFloat4x4( &mProj );
	XMMATRIX viewProj = view*proj;

	m_boxModel->Draw( viewProj, &mObjectConstantBuffer );
	m_gridModel->Draw( viewProj, &mObjectConstantBuffer );

	for ( int i = 0; i < 10; i++ )
	{
		m_cylinderModels[i]->Draw( viewProj, &mObjectConstantBuffer );
	}

	HR( mSwapChain->Present( 0, 0 ) );
}

void ShapesApp::OnMouseDown( WPARAM btnState, int x, int y )
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture( mhMainWnd );
}

void ShapesApp::OnMouseUp( WPARAM btnState, int x, int y )
{
	ReleaseCapture();
}

void ShapesApp::OnMouseMove( WPARAM btnState, int x, int y )
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

float ShapesApp::GetHeight( float x, float z )const
{
	return 0.3f*( z*sinf( 0.1f*x ) + x*cosf( 0.1f*z ) );
}

void ShapesApp::BuildGeometryBuffers()
{
	// Set up Box

	GeometryGenerator geoGen;

	GeometryGenerator::MeshData boxMesh;
	geoGen.CreateBox( 1.0f, 1.0f, 1.0f, boxMesh );

	// とりあえず頂点コピーしてるが、Vertex の定義をどこかにおいてそれを全体で使うか、頂点属性の使い分けをできるようにしたい
	XMFLOAT4 green( 0.0f, 0.8f, 0.0f, 1.0f );
	size_t count = boxMesh.Vertices.size();
	std::vector<Vertex> vertices( count );
	for ( size_t i = 0; i < count; i++ )
	{
		vertices[i].Position = boxMesh.Vertices[i].Position;
		vertices[i].Color = green;
	}

	Batch* boxBatch = new Batch( &md3dDevice, &md3dImmediateContext, &vertices, &boxMesh.Indices );
	m_boxModel = new Model( boxBatch );

	m_boxModel->SetTransition( XMFLOAT3( 0.0f, 0.5f, 0.0f ) );
	m_boxModel->SetScale( XMFLOAT3( 2.0f, 1.0f, 2.0f ) );


	// Set up Grid

	GeometryGenerator::MeshData gridMesh;
	geoGen.CreateGrid( 20.0f, 30.0f, 60, 40, gridMesh );

	count = gridMesh.Vertices.size();
	vertices.resize( count );
	for ( size_t i = 0; i < count; i++ )
	{
		vertices[i].Position = gridMesh.Vertices[i].Position;
		vertices[i].Color = green;
	}

	Batch* gridBatch = new Batch( &md3dDevice, &md3dImmediateContext, &vertices, &gridMesh.Indices );
	m_gridModel = new Model( gridBatch );


	// Set up Cylinder

	GeometryGenerator::MeshData cylinderMesh;
	geoGen.CreateCylinder( 0.5f, 0.3f, 3.0f, 20, 20, cylinderMesh );

	count = cylinderMesh.Vertices.size();
	vertices.resize( count );
	for ( size_t i = 0; i < count; i++ )
	{
		vertices[i].Position = cylinderMesh.Vertices[i].Position;
		vertices[i].Color = green;
	}

	Batch* cylinderBatch = new Batch( &md3dDevice, &md3dImmediateContext, &vertices, &cylinderMesh.Indices );
	for ( int i = 0; i < 10; i++ )
	{
		Model* cylinderModel = new Model( cylinderBatch );

		if ( i % 2 == 0 )
		{
			cylinderModel->SetTransition( XMFLOAT3( -5.0f, 1.5f, -10.0f + i*2.5f ) );
		}
		else
		{
			cylinderModel->SetTransition( XMFLOAT3( +5.0f, 1.5f, -10.0f + i*2.5f ) );
		}

		m_cylinderModels[i] = cylinderModel;

	}
}

void ShapesApp::BuildFX()
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	// Load cso files and create shaders
	HR( ShaderHelper::LoadCompiledShader( "SimplePixelShader.cso", &mPSBlob ) );
	HR( md3dDevice->CreatePixelShader( mPSBlob->GetBufferPointer(), mPSBlob->GetBufferSize(), NULL, &mPixelShader ) );

	HR( ShaderHelper::LoadCompiledShader( "SimpleVertexShader.cso", &mVSBlob ) );
	HR( md3dDevice->CreateVertexShader( mVSBlob->GetBufferPointer(), mVSBlob->GetBufferSize(), NULL, &mVertexShader ) );
}

void ShapesApp::BuildVertexLayout()
{
	// Create the vertex input layout.
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	HR( md3dDevice->CreateInputLayout( vertexDesc, 2, mVSBlob->GetBufferPointer(), mVSBlob->GetBufferSize(), &mInputLayout ) );
}

void ShapesApp::BuildRasterState()
{
	D3D11_RASTERIZER_DESC rs;
	memset( &rs, 0, sizeof( rs ) );
	rs.FillMode = D3D11_FILL_SOLID;
	rs.CullMode = D3D11_CULL_BACK;
	rs.AntialiasedLineEnable = rs.DepthClipEnable = true;
	mRasterState = NULL;
	HR( md3dDevice->CreateRasterizerState( &rs, &mRasterState ) );
}

void ShapesApp::BuildWireFrameRasterState()
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
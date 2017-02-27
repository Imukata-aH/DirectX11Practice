#include "d3dApp.h"
#include "d3dUtil.h"
#include "MathHelper.h"
#include "DirectXColors.h"
#include "cbPerObject.h"
#include "ConstantBuffer.h"
#include "ShaderHelper.h"
#include "GeometryGenerator.h"
using namespace DirectX;

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

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

private:
	ConstantBuffer<cbPerObject> mObjectConstantBuffer;
	ID3D11Buffer* mVB;
	ID3D11Buffer* mIB;
	ID3DBlob* mPSBlob;
	ID3DBlob* mVSBlob;
	ID3D11PixelShader* mPixelShader;
	ID3D11VertexShader* mVertexShader;
	ID3D11InputLayout* mInputLayout;
	ID3D11RasterizerState* mRasterState;

	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

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
	: D3DApp( hInstance ), mVB( 0 ), mIB( 0 ), mInputLayout( 0 ),
	mTheta( 1.5f*MathHelper::Pi ), mPhi( 0.25f*MathHelper::Pi ), mRadius( 5.0f )
{
	mMainWndCaption = L"Box Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4( &mWorld, I );
	XMStoreFloat4x4( &mView, I );
	XMStoreFloat4x4( &mProj, I );
}

ShapesApp::~ShapesApp()
{
	ReleaseCOM( mVB );
	ReleaseCOM( mIB );
	ReleaseCOM( mInputLayout );
	ReleaseCOM( mPSBlob );
	ReleaseCOM( mVSBlob );
}

bool ShapesApp::Init()
{
	if ( !D3DApp::Init() )
		return false;

	BuildGeometryBuffers();
	BuildFX();
	BuildVertexLayout();
	BuildRasterState();
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

	UINT stride = sizeof( Vertex );
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers( 0, 1, &mVB, &stride, &offset );
	md3dImmediateContext->IASetIndexBuffer( mIB, DXGI_FORMAT_R32_UINT, 0 );

	// Set constants
	XMMATRIX world = XMLoadFloat4x4( &mWorld );
	XMMATRIX view = XMLoadFloat4x4( &mView );
	XMMATRIX proj = XMLoadFloat4x4( &mProj );
	XMMATRIX worldViewProj = XMMatrixTranspose( world*view*proj );

	// Use a constant buffer. Effect framework deprecated
	cbPerObject mPerObjectCB;
	XMStoreFloat4x4( &mPerObjectCB.mWorldViewProj, worldViewProj );
	mObjectConstantBuffer.Data = mPerObjectCB;
	mObjectConstantBuffer.ApplyChanges( md3dImmediateContext );

	auto buffer = mObjectConstantBuffer.Buffer();
	md3dImmediateContext->VSSetConstantBuffers( 0, 1, &buffer );

	// Set raster state
	md3dImmediateContext->RSSetState( mRasterState );

	// Draw
	md3dImmediateContext->DrawIndexed( 36, 0, 0 );
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
		float dx = 0.005f*static_cast<float>( x - mLastMousePos.x );
		float dy = 0.005f*static_cast<float>( y - mLastMousePos.y );

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp( mRadius, 3.0f, 15.0f );
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void ShapesApp::BuildGeometryBuffers()
{
	GeometryGenerator geoGen;

	GeometryGenerator::MeshData mesh;
	geoGen.CreateGrid( 10.0f, 10.0f, 50, 50, mesh );
	//geoGen.CreateBox( 1.0f, 1.0f, 1.0f, mesh );

	std::vector<Vertex> vertices( mesh.Vertices.size() );
	for ( size_t i = 0; i < mesh.Vertices.size(); i++ )
	{
		vertices[i].Pos = mesh.Vertices[i].Position;
		vertices[i].Color = (const XMFLOAT4)Colors::White;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof( Vertex )*mesh.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR( md3dDevice->CreateBuffer( &vbd, &vinitData, &mVB ) );

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof( UINT )*mesh.Indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &mesh.Indices[0];
	HR( md3dDevice->CreateBuffer( &ibd, &iinitData, &mIB ) );
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
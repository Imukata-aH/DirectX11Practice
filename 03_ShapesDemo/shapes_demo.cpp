#include "d3dApp.h"
#include "d3dUtil.h"
#include "MathHelper.h"
#include "DirectXColors.h"
#include "cbPerObject.h"
#include "ConstantBuffer.h"
#include "ShaderHelper.h"
#include "GeometryGenerator.h"
using namespace DirectX;
using namespace DirectX::PackedVector;

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
	void BuildWireFrameRasterState();

	float GetHeight( float x, float z )const;

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

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	XMFLOAT4X4 mSphereWorld[10];
	XMFLOAT4X4 mCylWorld[10];
	XMFLOAT4X4 mBoxWorld;
	XMFLOAT4X4 mGridWorld;
	XMFLOAT4X4 mCenterShereWorld;

	int mBoxVertexOffset;
	int mGridVertexOffset;
	int mSphereVertexOffset;
	int mCylinderVertexOffset;

	UINT mBoxIndexOffset;
	UINT mGridIndexOffset;
	UINT mSphereIndexOffset;
	UINT mCylinderIndexOffset;

	UINT mBoxIndexCount;
	UINT mGridIndexCount;
	UINT mSphereIndexCount;
	UINT mCylinderIndexCount;

	int m_meshIndexCount;

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
	mTheta( 0.8f*MathHelper::Pi ), mPhi( 0.1f*MathHelper::Pi ), mRadius( 30.0f )
{
	mMainWndCaption = L"Shapes Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4( &mGridWorld, I );
	XMStoreFloat4x4( &mView, I );
	XMStoreFloat4x4( &mProj, I );

	XMMATRIX boxScale = XMMatrixScaling( 2.0f, 1.0f, 2.0f );
	XMMATRIX boxOffset = XMMatrixTranslation( 0.0f, 0.5f, 0.0f );
	XMStoreFloat4x4( &mBoxWorld, XMMatrixMultiply( boxScale, boxOffset ) );

	XMMATRIX centerSphereScale = XMMatrixScaling( 2.0f, 2.0f, 2.0f );
	XMMATRIX centerSphereOffset = XMMatrixTranslation( 0.0f, 2.0f, 0.0f );
	XMStoreFloat4x4( &mCenterShereWorld, XMMatrixMultiply( centerSphereScale, centerSphereOffset ) );

	for ( int i = 0; i < 5; ++i )
	{
		XMStoreFloat4x4( &mCylWorld[i * 2 + 0], XMMatrixTranslation( -5.0f, 1.5f, -10.0f + i*5.0f ) );
		XMStoreFloat4x4( &mCylWorld[i * 2 + 1], XMMatrixTranslation( +5.0f, 1.5f, -10.0f + i*5.0f ) );

		XMStoreFloat4x4( &mSphereWorld[i * 2 + 0], XMMatrixTranslation( -5.0f, 3.5f, -10.0f + i*5.0f ) );
		XMStoreFloat4x4( &mSphereWorld[i * 2 + 1], XMMatrixTranslation( +5.0f, 3.5f, -10.0f + i*5.0f ) );
	}
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

	UINT stride = sizeof( Vertex );
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers( 0, 1, &mVB, &stride, &offset );
	md3dImmediateContext->IASetIndexBuffer( mIB, DXGI_FORMAT_R32_UINT, 0 );

	// Set constants
	XMMATRIX view = XMLoadFloat4x4( &mView );
	XMMATRIX proj = XMLoadFloat4x4( &mProj );
	XMMATRIX viewProj = view*proj;
	//XMMATRIX worldViewProj = XMMatrixTranspose( world*view*proj );

	XMMATRIX worldViewProj;
	cbPerObject mPerObjectCB;

	// TODO: Constant buffer の更新処理を一箇所にまとめる

	// Draw the grid.
	XMMATRIX world = XMLoadFloat4x4( &mGridWorld );
	worldViewProj = XMMatrixTranspose( world*view*proj );
	XMStoreFloat4x4( &mPerObjectCB.mWorldViewProj, worldViewProj );
	mObjectConstantBuffer.Data = mPerObjectCB;
	mObjectConstantBuffer.ApplyChanges( md3dImmediateContext );

	auto buffer = mObjectConstantBuffer.Buffer();
	md3dImmediateContext->VSSetConstantBuffers( 0, 1, &buffer );
	md3dImmediateContext->DrawIndexed( mGridIndexCount, mGridIndexOffset, mGridVertexOffset );

	// Draw the box.
	world = XMLoadFloat4x4( &mBoxWorld );
	worldViewProj = XMMatrixTranspose( world*view*proj );
	XMStoreFloat4x4( &mPerObjectCB.mWorldViewProj, worldViewProj );
	mObjectConstantBuffer.Data = mPerObjectCB;
	mObjectConstantBuffer.ApplyChanges( md3dImmediateContext );

	buffer = mObjectConstantBuffer.Buffer();
	md3dImmediateContext->VSSetConstantBuffers( 0, 1, &buffer );
	md3dImmediateContext->DrawIndexed( mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset );

	// Draw center sphere.
	world = XMLoadFloat4x4( &mCenterShereWorld );
	worldViewProj = XMMatrixTranspose( world*view*proj );
	XMStoreFloat4x4( &mPerObjectCB.mWorldViewProj, worldViewProj );
	mObjectConstantBuffer.Data = mPerObjectCB;
	mObjectConstantBuffer.ApplyChanges( md3dImmediateContext );

	buffer = mObjectConstantBuffer.Buffer();
	md3dImmediateContext->VSSetConstantBuffers( 0, 1, &buffer );
	md3dImmediateContext->DrawIndexed( mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset );

	// Draw the cylinders.
	for ( int i = 0; i < 10; ++i )
	{
		world = XMLoadFloat4x4( &mCylWorld[i] );
		worldViewProj = XMMatrixTranspose( world*view*proj );
		XMStoreFloat4x4( &mPerObjectCB.mWorldViewProj, worldViewProj );
		mObjectConstantBuffer.Data = mPerObjectCB;
		mObjectConstantBuffer.ApplyChanges( md3dImmediateContext );

		auto buffer = mObjectConstantBuffer.Buffer();
		md3dImmediateContext->VSSetConstantBuffers( 0, 1, &buffer );
		md3dImmediateContext->DrawIndexed( mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset );
	}

	// Draw the spheres.
	for ( int i = 0; i < 10; ++i )
	{
		world = XMLoadFloat4x4( &mSphereWorld[i] );
		worldViewProj = XMMatrixTranspose( world*view*proj );
		XMStoreFloat4x4( &mPerObjectCB.mWorldViewProj, worldViewProj );
		mObjectConstantBuffer.Data = mPerObjectCB;
		mObjectConstantBuffer.ApplyChanges( md3dImmediateContext );

		auto buffer = mObjectConstantBuffer.Buffer();
		md3dImmediateContext->VSSetConstantBuffers( 0, 1, &buffer );
		md3dImmediateContext->DrawIndexed( mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset );
	}

	// Set raster state
	md3dImmediateContext->RSSetState( mRasterState );

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
	GeometryGenerator::MeshData box;
	GeometryGenerator::MeshData grid;
	GeometryGenerator::MeshData sphere;
	GeometryGenerator::MeshData cylinder;

	GeometryGenerator geoGen;
	geoGen.CreateBox( 1.0f, 1.0f, 1.0f, box );
	geoGen.CreateGrid( 20.0f, 30.0f, 60, 40, grid );
	geoGen.CreateSphere( 0.5f, 20, 20, sphere );
	//geoGen.CreateGeosphere(0.5f, 2, sphere);
	geoGen.CreateCylinder( 0.5f, 0.3f, 3.0f, 20, 20, cylinder );

	mBoxVertexOffset = 0;
	mGridVertexOffset = box.Vertices.size();
	mSphereVertexOffset = mGridVertexOffset + grid.Vertices.size();
	mCylinderVertexOffset = mSphereVertexOffset + sphere.Vertices.size();

	mBoxIndexCount = box.Indices.size();
	mGridIndexCount = grid.Indices.size();
	mSphereIndexCount = sphere.Indices.size();
	mCylinderIndexCount = cylinder.Indices.size();

	mBoxIndexOffset = 0;
	mGridIndexOffset = mBoxIndexOffset;
	mSphereIndexOffset = mGridIndexOffset + mGridIndexCount;
	mCylinderIndexOffset = mSphereIndexOffset + mSphereIndexCount;

	UINT totalVertexCount =
		box.Vertices.size() +
		grid.Vertices.size() +
		sphere.Vertices.size() +
		cylinder.Vertices.size();

	UINT totalIndexCount =
		mBoxIndexCount +
		mGridIndexCount +
		mSphereIndexCount +
		mCylinderIndexCount;

	std::vector<Vertex> vertices( totalVertexCount );

	XMFLOAT4 black( 0.0f, 0.0f, 0.0f, 1.0f );

	UINT k = 0;
	for ( size_t i = 0; i < box.Vertices.size(); ++i, ++k )
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Color = black;
	}

	for ( size_t i = 0; i < grid.Vertices.size(); ++i, ++k )
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Color = black;
	}

	for ( size_t i = 0; i < sphere.Vertices.size(); ++i, ++k )
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Color = black;
	}

	for ( size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k )
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Color = black;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof( Vertex ) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR( md3dDevice->CreateBuffer( &vbd, &vinitData, &mVB ) );

	std::vector<UINT> indices;
	indices.insert( indices.end(), box.Indices.begin(), box.Indices.end() );
	indices.insert( indices.end(), grid.Indices.begin(), grid.Indices.end() );
	indices.insert( indices.end(), sphere.Indices.begin(), sphere.Indices.end() );
	indices.insert( indices.end(), cylinder.Indices.begin(), cylinder.Indices.end() );

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof( UINT ) * totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
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

void ShapesApp::BuildWireFrameRasterState()
{
	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory( &wireframeDesc, sizeof( D3D11_RASTERIZER_DESC ) );
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;
	mRasterState = NULL;
	HR( md3dDevice->CreateRasterizerState( &wireframeDesc, &mRasterState ) );
}
#include "MyRender.h"

struct cbMatrixData
{
	XMMATRIX WVP;
	XMMATRIX World;
};

struct cbLightData
{
	XMFLOAT4 ambientColor;
	XMFLOAT4 diffuseColor;
	XMFLOAT3 lightDirection;
	float pad;
};

struct Vertex
{
	Vertex(float x, float y, float z, float u, float v, float nx,
	float ny, float nz) : pos(x, y, z), tex(u, v), normal(nx, ny, nz){}

	XMFLOAT3 pos;
	XMFLOAT2 tex;
	XMFLOAT3 normal;
};


MyRender::MyRender()
{
	IndexBuffer = nullptr;
	VertBuffer = nullptr;
	constMatrixBuffer = nullptr;
	constLightBuffer = nullptr;
	shader = nullptr;
}

bool MyRender::Init()
{
	shader = new Shader(this);
	if(!shader) return false;

	if(!shader->LoadTexture(L"Grass.png")) return false;

	shader->AddInputElementDesc("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	shader->AddInputElementDesc("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	shader->AddInputElementDesc("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);

	if(!shader->CreateShader(L"shader.vs", L"shader.ps")) return false;

	//Нормали вершин немного не верны. т.к. по идее у каждой вершины их 3.
	Vertex v[] =
	{
		Vertex(-1.0f, -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, -1.0f, -1.0f),
		Vertex(-1.0f,  1.0f, -1.0f, 0.0f, 0.0f, -1.0f,  1.0f, -1.0f),
		Vertex( 1.0f,  1.0f, -1.0f, 1.0f, 0.0f,  1.0f,  1.0f, -1.0f),
		Vertex( 1.0f, -1.0f, -1.0f, 1.0f, 1.0f,  1.0f, -1.0f, -1.0f),

		Vertex(-1.0f, -1.0f,  1.0f, 1.0f, 1.0f, -1.0f, -1.0f,  1.0f),
		Vertex( 1.0f, -1.0f,  1.0f, 0.0f, 1.0f,  1.0f, -1.0f,  1.0f),
		Vertex( 1.0f,  1.0f,  1.0f, 0.0f, 0.0f,  1.0f,  1.0f,  1.0f),
		Vertex(-1.0f,  1.0f,  1.0f, 1.0f, 0.0f, -1.0f,  1.0f,  1.0f),

		Vertex(-1.0f,  1.0f, -1.0f, 0.0f, 1.0f, -1.0f,  1.0f, -1.0f),
		Vertex(-1.0f,  1.0f,  1.0f, 0.0f, 0.0f, -1.0f,  1.0f,  1.0f),
		Vertex( 1.0f,  1.0f,  1.0f, 1.0f, 0.0f,  1.0f,  1.0f,  1.0f),
		Vertex( 1.0f,  1.0f, -1.0f, 1.0f, 1.0f,  1.0f,  1.0f, -1.0f),

		Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f),
		Vertex( 1.0f, -1.0f, -1.0f, 0.0f, 1.0f,  1.0f, -1.0f, -1.0f),
		Vertex( 1.0f, -1.0f,  1.0f, 0.0f, 0.0f,  1.0f, -1.0f,  1.0f),
		Vertex(-1.0f, -1.0f,  1.0f, 1.0f, 0.0f, -1.0f, -1.0f,  1.0f),

		Vertex(-1.0f, -1.0f,  1.0f, 0.0f, 1.0f, -1.0f, -1.0f,  1.0f),
		Vertex(-1.0f,  1.0f,  1.0f, 0.0f, 0.0f, -1.0f,  1.0f,  1.0f), //
		Vertex(-1.0f,  1.0f, -1.0f, 1.0f, 0.0f, -1.0f,  1.0f, -1.0f), //
		Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f), //

		Vertex( 1.0f, -1.0f, -1.0f, 0.0f, 1.0f,  1.0f, -1.0f, -1.0f), //
		Vertex( 1.0f,  1.0f, -1.0f, 0.0f, 0.0f,  1.0f,  1.0f, -1.0f),
		Vertex( 1.0f,  1.0f,  1.0f, 1.0f, 0.0f,  1.0f,  1.0f,  1.0f), //
		Vertex( 1.0f, -1.0f,  1.0f, 1.0f, 1.0f,  1.0f, -1.0f,  1.0f)
	};

	DWORD indices[] = 
	{
		0,1,2,0,2,3,
		4,5,6,4,6,7,
		8,9,10,8,10,11,
		12,13,14,12,14,15,
		16,17,18,16,18,19,
		20,21,22,20,22,23
	};

	IndexBuffer = Buffer::CreateIndexBuffer(m_pd3dDevice, sizeof(DWORD)*36, false, indices);
	VertBuffer = Buffer::CreateVertexBuffer(m_pd3dDevice, sizeof(Vertex)*24, false, v);
	constMatrixBuffer = Buffer::CreateConstantBuffer(m_pd3dDevice, sizeof(cbMatrixData), false);
	constLightBuffer = Buffer::CreateConstantBuffer(m_pd3dDevice, sizeof(cbLightData), false);

	XMVECTOR camPosition = XMVectorSet(0.0f, 3.0f, -8.0f, 0.0f);
	XMVECTOR camTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);

	return true;
}

bool MyRender::Draw()
{
	static float rot = 0.0f;
	DWORD dwTimeCur = GetTickCount();
	static DWORD dwTimeStart = dwTimeCur; 
	
	rot = (dwTimeCur - dwTimeStart) / 1000.0f;

	XMMATRIX cube1World = XMMatrixIdentity();
	XMVECTOR rotaxis = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX Rotation = XMMatrixRotationAxis(rotaxis, rot);
	cube1World = Rotation;

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pImmediateContext->IASetVertexBuffers(0, 1, &VertBuffer, &stride, &offset);
	m_pImmediateContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	cbLightData cblgh;
	cblgh.ambientColor = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);
	cblgh.diffuseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	cblgh.lightDirection = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_pImmediateContext->UpdateSubresource(constLightBuffer, 0, NULL, &cblgh, 0, 0);
	m_pImmediateContext->PSSetConstantBuffers(0, 1, &constLightBuffer);

	XMMATRIX WVP = cube1World * camView * m_Projection;

	cbMatrixData cbMat;
	cbMat.World = XMMatrixTranspose(cube1World);
	cbMat.WVP = XMMatrixTranspose(WVP);
	m_pImmediateContext->UpdateSubresource(constMatrixBuffer, 0, NULL, &cbMat, 0, 0);
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &constMatrixBuffer);

	shader->Draw();
	m_pImmediateContext->DrawIndexed(36, 0, 0);

	/*XMVECTOR lightVector = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	//Перемножает вектор на матрицу (w у вектора(in и out) = 1.0).
	lightVector = XMVector3TransformCoord(lightVector, cube1World);

	light.pos.x = XMVectorGetX(lightVector); 
	light.pos.y = XMVectorGetY(lightVector);
	light.pos.z = XMVectorGetZ(lightVector);
	//XMStoreFloat3(&light.pos, XMVector3TransformCoord(lightVector, cube1World));

	XMMATRIX cube2World = XMMatrixIdentity();
	Rotation = XMMatrixRotationAxis(rotaxis, -rot);
	Scale = XMMatrixScaling(1.3f, 1.3f, 1.3f);
	cube2World = Rotation * Scale;

	
	WVP = cube2World * camView * m_Projection;
	cbMat.World = XMMatrixTranspose(cube2World);
	cbMat.WVP = XMMatrixTranspose(WVP);
	m_pImmediateContext->UpdateSubresource(constMatrixBuffer, 0, NULL, &cbMat, 0, 0);
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &constMatrixBuffer);
	shader->Draw();
	m_pImmediateContext->DrawIndexed(36, 0, 0);*/

	return true;
}

void MyRender::Close()
{
	_CLOSE(shader);
	_RELEASE(IndexBuffer);
	_RELEASE(VertBuffer);
	_RELEASE(constMatrixBuffer);
	_RELEASE(constLightBuffer);
}


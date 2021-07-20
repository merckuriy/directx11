#include "MyRender.h"

struct cbMatrixData
{
	XMMATRIX WVP;
	XMMATRIX World;
};

//Выравнивание структуры (буфера) по 16 байт (_declspec(align(16))
//работает неправильно, поэтому дополним её float pad1.
 struct Light
{
	Light(){ ZeroMemory(this, sizeof(Light)); }

	XMFLOAT3 pos;//Позиция света в мировых координатах.
	float range; //Дальность распространения.
	XMFLOAT3 att; //Коэффициенты затухания.
	float pad1;// для выравнивания до 16.

	XMFLOAT4 ambient; //Коэффициент фонового света
	XMFLOAT4 diffuse; //Интенсивность света.
} light;

struct cbLightData
{
	Light light;
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

	if(!shader->CreateShader(L"pointlight.vs", L"pointlight.ps")) return false;

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

	light.pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	light.range = 100.0f;
	light.att = XMFLOAT3(0.0f, 0.2f, 0.0f);
	light.ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f); 
	light.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	return true;
}

bool MyRender::Draw()
{
	static float rot = 0.0f;
	DWORD dwTimeCur = GetTickCount();
	static DWORD dwTimeStart = dwTimeCur; //=0
	
	rot = (dwTimeCur - dwTimeStart) / 1000.0f;

	XMMATRIX cube1World = XMMatrixIdentity();
	XMVECTOR rotaxis = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX Rotation = XMMatrixRotationAxis(rotaxis, rot);
	XMMATRIX Scale = XMMatrixScaling(0.4f, 0.4f, 0.4f);
	XMMATRIX Translation = XMMatrixTranslation(0.0f, 0.0f, 4.0f);
	cube1World = Scale * Translation * Rotation;

	
	XMVECTOR lightVector = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
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

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pImmediateContext->IASetVertexBuffers(0, 1, &VertBuffer, &stride, &offset);
	m_pImmediateContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	cbLightData cblgh;
	cblgh.light = light;
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

	WVP = cube2World * camView * m_Projection;
	cbMat.World = XMMatrixTranspose(cube2World);
	cbMat.WVP = XMMatrixTranspose(WVP);
	m_pImmediateContext->UpdateSubresource(constMatrixBuffer, 0, NULL, &cbMat, 0, 0);
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &constMatrixBuffer);
	shader->Draw();
	m_pImmediateContext->DrawIndexed(36, 0, 0);

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


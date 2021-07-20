#include "MyRender.h"

struct cbMatrixData
{
	XMMATRIX WVP;
	XMMATRIX World;
};

/*_declspec(align(16))*/ struct Light
{
	Light(){ ZeroMemory(this, sizeof(Light)); }

	XMFLOAT3 pos; //Позиция света в мировых координатах.
	float range; //Дальность распространения.
	XMFLOAT3 dir; //Направление света.
	float exponent; //Степень затухания к краям.
	XMFLOAT3 att; //Коэффициенты затухания.
	float pad1;// для выравнивания до 16.
	XMFLOAT4 ambient; //Коэффициент фонового света.
	XMFLOAT4 diffuse; //Интенсивность света.
	float lightCos; //Косинус угла света.
	XMFLOAT3 pad2;

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

	if(!shader->LoadTexture(L"wood.jpg")) return false; //metal.jpg wood.jpg Grass.png

	shader->AddInputElementDesc("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	shader->AddInputElementDesc("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	shader->AddInputElementDesc("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);

	if(!shader->CreateShader(L"pointlight.vs", L"pointlight.ps")) return false;

	Vertex v[] =
	{
		Vertex(-1.0f, -1.0f, -1.0f, 0.0f, 1.0f,  0.0f,  0.0f, -1.0f),
		Vertex(-1.0f,  1.0f, -1.0f, 0.0f, 0.0f,  0.0f,  0.0f, -1.0f),
		Vertex( 1.0f,  1.0f, -1.0f, 1.0f, 0.0f,  0.0f,  0.0f, -1.0f),
		Vertex( 1.0f, -1.0f, -1.0f, 1.0f, 1.0f,  0.0f,  0.0f, -1.0f),

		Vertex(-1.0f, -1.0f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f,  1.0f),
		Vertex( 1.0f, -1.0f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,  1.0f),
		Vertex( 1.0f,  1.0f,  1.0f, 0.0f, 0.0f,  0.0f,  0.0f,  1.0f),
		Vertex(-1.0f,  1.0f,  1.0f, 1.0f, 0.0f,  0.0f,  0.0f,  1.0f),

		Vertex(-1.0f,  1.0f, -1.0f, 0.0f, 1.0f,  0.0f,  1.0f,  0.0f),
		Vertex(-1.0f,  1.0f,  1.0f, 0.0f, 0.0f,  0.0f,  1.0f,  0.0f),
		Vertex( 1.0f,  1.0f,  1.0f, 1.0f, 0.0f,  0.0f,  1.0f,  0.0f),
		Vertex( 1.0f,  1.0f, -1.0f, 1.0f, 1.0f,  0.0f,  1.0f,  0.0f),

		Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f,  0.0f, -1.0f,  0.0f),
		Vertex( 1.0f, -1.0f, -1.0f, 0.0f, 1.0f,  0.0f, -1.0f,  0.0f),
		Vertex( 1.0f, -1.0f,  1.0f, 0.0f, 0.0f,  0.0f, -1.0f,  0.0f),
		Vertex(-1.0f, -1.0f,  1.0f, 1.0f, 0.0f,  0.0f, -1.0f,  0.0f),

		Vertex(-1.0f, -1.0f,  1.0f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f),
		Vertex(-1.0f,  1.0f,  1.0f, 0.0f, 0.0f, -1.0f,  0.0f,  0.0f), 
		Vertex(-1.0f,  1.0f, -1.0f, 1.0f, 0.0f, -1.0f,  0.0f,  0.0f), 
		Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f,  0.0f,  0.0f), 

		Vertex( 1.0f, -1.0f, -1.0f, 0.0f, 1.0f,  1.0f,  0.0f,  0.0f), 
		Vertex( 1.0f,  1.0f, -1.0f, 0.0f, 0.0f,  1.0f,  0.0f,  0.0f),
		Vertex( 1.0f,  1.0f,  1.0f, 1.0f, 0.0f,  1.0f,  0.0f,  0.0f), 
		Vertex( 1.0f, -1.0f,  1.0f, 1.0f, 1.0f,  1.0f,  0.0f,  0.0f)
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

	XMVECTOR camPosition = XMVectorSet(0.0f, 4.0f, -5.0f, 0.0f);
	XMVECTOR camTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);

	light.range = 8.0f; 
	light.att = XMFLOAT3(0.0f, 0.1f, 0.02f);
	light.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f); 
	light.diffuse = XMFLOAT4(2.0f, 2.0f, 2.0f, 1.0f);
	light.lightCos = 0.95f;
	light.exponent = 1.8f;

	return true;
}

bool MyRender::Draw()
{
	static float rot = 0.0f;
	DWORD dwTimeCur = GetTickCount();
	static DWORD dwTimeStart = dwTimeCur;
	
	rot = (dwTimeCur - dwTimeStart) / 1000.0f;

	light.pos = XMFLOAT3(3.0f, 1.5f, 3.0f);

	//Направлен на центр системы координат.
	//light.dir = XMFLOAT3(-light.pos.x, -light.pos.y, -light.pos.z);
	light.dir = XMFLOAT3(-3.0f, -0.5f, -3.0f);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pImmediateContext->IASetVertexBuffers(0, 1, &VertBuffer, &stride, &offset);
	m_pImmediateContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	cbLightData cblgh;
	cblgh.light = light;
	m_pImmediateContext->UpdateSubresource(constLightBuffer, 0, NULL, &cblgh, 0, 0);
	m_pImmediateContext->PSSetConstantBuffers(0, 1, &constLightBuffer);

	XMMATRIX cube1World = XMMatrixScaling(0.2f, 0.2f, 0.2f)*
		XMMatrixTranslation(light.pos.x, light.pos.y, light.pos.z);
	XMMATRIX WVP = cube1World * camView * m_Projection;
	cbMatrixData cbMat;
	cbMat.World = XMMatrixTranspose(cube1World);
	cbMat.WVP = XMMatrixTranspose(WVP);
	m_pImmediateContext->UpdateSubresource(constMatrixBuffer, 0, NULL, &cbMat, 0, 0);
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &constMatrixBuffer);
	shader->Draw();
	m_pImmediateContext->DrawIndexed(36, 0, 0);

	// Пол из 9 кубов.
	XMMATRIX cubeWorld[] = {
		XMMatrixTranslation(-2.0f, 0.0f,  2.0f),
		XMMatrixTranslation(-2.0f, 0.0f,  0.0f),
		XMMatrixTranslation(-2.0f, 0.0f, -2.0f),
		XMMatrixTranslation( 0.0f, 0.0f,  2.0f),
		XMMatrixTranslation( 0.0f, 0.0f,  0.0f),
		XMMatrixTranslation( 0.0f, 0.0f, -2.0f),
		XMMatrixTranslation( 2.0f, 0.0f,  2.0f),
		XMMatrixTranslation( 2.0f, 0.0f,  0.0f),
		XMMatrixTranslation( 2.0f, 0.0f, -2.0f)
	};

	for(unsigned short i = 0; i < ARRAYSIZE(cubeWorld); i++){
		// Немного уменьшаем толщину пола
		WVP = XMMatrixScaling(1.0f, 0.2f, 1.0f) * cubeWorld[i] * camView * m_Projection;
		cbMat.World = XMMatrixTranspose(cubeWorld[i]);
		cbMat.WVP = XMMatrixTranspose(WVP);
		m_pImmediateContext->UpdateSubresource(constMatrixBuffer, 0, NULL, &cbMat, 0, 0);
		m_pImmediateContext->VSSetConstantBuffers(0, 1, &constMatrixBuffer);
		shader->Draw();
		m_pImmediateContext->DrawIndexed(36, 0, 0);
	}

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


#include "MyRender.h"

std::wstring intToStr(int i)
{
	wchar_t str[255];
	swprintf(str, 255, L"%d", i);
	return str;
}

struct cbMatrixData
{
	XMMATRIX WVP;
	XMMATRIX World;
};

/*_declspec(align(16))*/ struct Light
{
	Light(){ ZeroMemory(this, sizeof(Light)); }

	XMFLOAT3 pos;
	float range;
	XMFLOAT3 dir;
	float exponent;
	XMFLOAT3 att;
	float pad1;
	XMFLOAT4 ambient; 
	XMFLOAT4 diffuse;
	float lightCos;
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

	if(!shader->LoadTexture(L"Grass.png")) return false;

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

	XMVECTOR camPosition = XMVectorSet(0.0f, 4.0f, -8.0f, 0.0f);
	XMVECTOR camTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);

	light.range = 8.0f; 
	light.att = XMFLOAT3(0.0f, 0.1f, 0.1f);
	light.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f); 
	light.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	light.lightCos = 0.85f; //0.95f
	light.exponent = 1.8f;

	//Создаём шрифт с помощью font.fnt
	m_font = new BitmapFont(this);
	if(!m_font->Init("font.fnt")) return false;
	text1 = new Text(this, m_font);
	text1->Init(L"\"Динамический текст\"", true, 30);

	m_font2 = new BitmapFont(this);
	if(!m_font2->Init("fontITALIC.fnt")) return false; //fontBOLD
	text2 = new Text(this, m_font2);
	text2->Init(L"\"Статический текст\"");

	fps.Init();

	return true;
}

bool MyRender::Draw()
{
	fps.Frame(); //добавляем кадр.

	TurnZBufferOff();
	TurnOnAlphaBlending(); //Включаем прозрачность

	text1->SetText(L"FPS: " + intToStr(fps.GetFps()));
	text1->Draw(0.0f, 1.0f, 0.0f, 0.0f, 10.0f);
	text2->Draw(1.0f, 1.0f, 0.0f, 10.0f, m_height-35.0f);

	TurnOffAlphaBlending();
	TurnZBufferOn();

	static float rot = 0.0f;
	DWORD dwTimeCur = GetTickCount();
	static DWORD dwTimeStart = dwTimeCur;
	rot = (dwTimeCur - dwTimeStart) / 1000.0f;

	light.pos = XMFLOAT3(0.0f, 4.0f, 0.0f); //3.0f, 1.5f, 3.0f
	light.dir = XMFLOAT3(-light.pos.x, -light.pos.y, -light.pos.z);
	//light.dir = XMFLOAT3(-3.0f, -0.5f, -3.0f);

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
		XMMatrixTranslation( 0.0f, 1.0f,  0.0f),
		XMMatrixTranslation( 0.0f, 0.0f, -2.0f),
		XMMatrixTranslation( 2.0f, 0.0f,  2.0f),
		XMMatrixTranslation( 2.0f, 0.0f,  0.0f),
		XMMatrixTranslation( 2.0f, 0.0f, -2.0f)
	};

	for(unsigned short i = 0; i < ARRAYSIZE(cubeWorld); i++){
		WVP = cubeWorld[i] * camView * m_Projection;
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
	_CLOSE(m_font);
	_CLOSE(text1);
	_CLOSE(m_font2);
	_CLOSE(text2);
	_RELEASE(IndexBuffer);
	_RELEASE(VertBuffer);
	_RELEASE(constMatrixBuffer);
	_RELEASE(constLightBuffer);
}


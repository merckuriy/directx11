#include "MyRender.h"

struct Vertex
{
	Vertex(float x, float y, float z, float u, float v) : pos(x, y, z), tex(u, v){}

	XMFLOAT3 pos;
	XMFLOAT2 tex;
};

struct ConstantBuffer
{
	XMMATRIX WVP;
};

MyRender::MyRender()
{
	m_vertexBuffer = nullptr;
	m_indexBuffer = nullptr;
	m_constantBuffer = nullptr;
	m_shader = nullptr;
};


bool MyRender::Init()
{
	// геометрия (куб)
	Vertex vert[] =
	{
		Vertex(-1.0f, -1.0f, -1.0f, 0.0f, 1.0f),
		Vertex(-1.0f,  1.0f, -1.0f, 0.0f, 0.0f),
		Vertex( 1.0f,  1.0f, -1.0f, 1.0f, 0.0f),
		Vertex( 1.0f, -1.0f, -1.0f, 1.0f, 1.0f),

		Vertex(-1.0f, -1.0f,  1.0f, 1.0f, 1.0f),
		Vertex( 1.0f, -1.0f,  1.0f, 0.0f, 1.0f),
		Vertex( 1.0f,  1.0f,  1.0f, 0.0f, 0.0f),
		Vertex(-1.0f,  1.0f,  1.0f, 1.0f, 0.0f),

		Vertex(-1.0f,  1.0f, -1.0f, 0.0f, 1.0f),
		Vertex(-1.0f,  1.0f,  1.0f, 0.0f, 0.0f),
		Vertex( 1.0f,  1.0f,  1.0f, 1.0f, 0.0f),
		Vertex( 1.0f,  1.0f, -1.0f, 1.0f, 1.0f),

		Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f),
		Vertex( 1.0f, -1.0f, -1.0f, 0.0f, 1.0f),
		Vertex( 1.0f, -1.0f,  1.0f, 0.0f, 0.0f),
		Vertex(-1.0f, -1.0f,  1.0f, 1.0f, 0.0f),

		Vertex(-1.0f, -1.0f,  1.0f, 0.0f, 1.0f),
		Vertex(-1.0f,  1.0f,  1.0f, 0.0f, 0.0f),
		Vertex(-1.0f,  1.0f, -1.0f, 1.0f, 0.0f),
		Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f),

		Vertex( 1.0f, -1.0f, -1.0f, 0.0f, 1.0f),
		Vertex( 1.0f,  1.0f, -1.0f, 0.0f, 0.0f),
		Vertex( 1.0f,  1.0f,  1.0f, 1.0f, 0.0f),
		Vertex( 1.0f, -1.0f,  1.0f, 1.0f, 1.0f),
	};

	unsigned long indices[] = { 
		0,1,2,  0,2,3,
		4,5,6,  4,6,7,
		8,9,10, 8,10,11,
		12,13,14, 12,14,15,
		16,17,18, 16,18,19,
		20,21,22, 20,22,23 
	};

	m_vertexBuffer = Buffer::CreateVertexBuffer(m_pd3dDevice, sizeof(Vertex)*24, false, vert);
	m_indexBuffer = Buffer::CreateIndexBuffer(m_pd3dDevice, sizeof(unsigned long)*36, false, indices);
	m_constantBuffer = Buffer::CreateConstantBuffer(m_pd3dDevice, sizeof(ConstantBuffer), false);

	//Shader and input format initialization.
	m_shader = new Shader(this);
	m_shader->AddInputElementDesc("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	m_shader->AddInputElementDesc("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	if (!m_shader->CreateShader(L"shader.vs", L"shader.ps")) return false;
	if (!m_shader->AddTexture(L"text1.jpg")) return false;

	//Шейдер для render target текстуры. Но т.к. файлы шейдеров те же, его можно не использовать.
	//m_shaderRT = new Shader(this);
	//m_shaderRT->AddInputElementDesc("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	//m_shaderRT->AddInputElementDesc("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	//if (!m_shaderRT->CreateShader(L"shader.vs", L"shader.ps")) return false;

	//Camera setting 
	XMVECTOR camPosition = XMVectorSet(0.0f, 1.5f, -3.5f, 0.0f);
	XMVECTOR camTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);

	if (!m_rt.Init(m_pd3dDevice, 640, 480)) return false;

	if (!m_rw.Init(m_pd3dDevice, 640, 480, 160, 120)) return false;

	return true;
}

bool MyRender::Draw()
{
	// Чёрный фон для сцены
	float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, color);
	m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	m_shader->Draw(); // Шейдер достаточно проставить один раз за кадр
	RenderScene();

	TurnZBufferOff();	
	RenderTexture(); //Рисуем в нашу текстуру
	TurnZBufferOn();

	return true;
}

void MyRender::RenderScene()
{
	unsigned int stride = sizeof(Vertex);
	unsigned int offset = 0;
	static float rotation = 0.0f;
	rotation += XM_PI*0.00005f;
	if (rotation >= 360.0f) rotation = 0.0f; //-= 360.0f

	XMMATRIX world = XMMatrixRotationY(rotation);
	ConstantBuffer cb;
	cb.WVP = XMMatrixTranspose(world*camView*m_Projection);

	m_pImmediateContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	m_pImmediateContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_pImmediateContext->UpdateSubresource(m_constantBuffer, 0, NULL, &cb, 0, 0);
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_constantBuffer);
	//m_shader->Draw();
	m_pImmediateContext->DrawIndexed(36, 0, 0);
}

void MyRender::RenderTexture()
{
	//Указываем текстуру для рисования
	m_rt.SetRenderTarget(m_pImmediateContext, m_pDepthStencilView);
	// Сиреневый фон для текстуры
	m_rt.ClearRenderTarget(m_pImmediateContext, m_pDepthStencilView, 0.5f, 0.5f, 1.0f, 1.0f);
	RenderScene(); //Рисуем сцену уже в текстуру
	//Опять устанавливаем основной render target view (в back buffer)
	m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	//Рисуем полученную выше текстуру как обычное двухмерное изображение
	XMMATRIX objmatrix = XMMatrixTranslation(10.0f, -10.0f, 0.0f);
	ConstantBuffer cb;
	cb.WVP = XMMatrixTranspose(objmatrix * m_Ortho);
	m_pImmediateContext->UpdateSubresource(m_constantBuffer, 0, NULL, &cb, 0, 0);
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_constantBuffer);
	//m_shaderRT->Draw();

	ID3D11ShaderResourceView *texResource = m_rt.GetShaderResourceView();
	m_pImmediateContext->PSSetShaderResources(0, 1, &texResource);
	// Рисуем текстуру в окне
	m_rw.Render(m_pImmediateContext);
}



void MyRender::Close()
{
	_RELEASE(m_vertexBuffer);
	_RELEASE(m_indexBuffer);
	_RELEASE(m_constantBuffer);
	_CLOSE(m_shader);
	//_CLOSE(m_shaderRT);
}


#include "MyRender.h"

struct Vertex
{
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
	// геометрия (квадрат)
	Vertex vert[] =
	{
		{ XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3( 1.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f,-1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3( 1.0f,-1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) }
	};

	unsigned long indices[6] = { 0,1,2, 2,1,3 };

	m_vertexBuffer = Buffer::CreateVertexBuffer(m_pd3dDevice, sizeof(Vertex)*4, false, &vert);
	m_indexBuffer = Buffer::CreateIndexBuffer(m_pd3dDevice, sizeof(unsigned long)*6, false, &indices);
	m_constantBuffer = Buffer::CreateConstantBuffer(m_pd3dDevice, sizeof(ConstantBuffer), false);

	//Shader and input format initialization.
	m_shader = new Shader(this);
	m_shader->AddInputElementDesc("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	m_shader->AddInputElementDesc("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	if (!m_shader->CreateShader(L"alphamap.vs", L"alphamap.ps")) return false;
	if (!m_shader->AddTexture(L"text1.png") || 
		!m_shader->AddTexture(L"text2.png") ||
		!m_shader->AddTexture(L"alpha.png")) return false;

	//Camera setting 
	XMVECTOR camPosition = XMVectorSet(0.0f, 0.0f, -1.5f, 0.0f);
	XMVECTOR camTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);

	return true;
}

bool MyRender::Draw()
{
	unsigned int stride = sizeof(Vertex);
	unsigned int offset = 0;
	ConstantBuffer cb;
	cb.WVP = XMMatrixTranspose(camView*m_Projection);

	m_pImmediateContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	m_pImmediateContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_pImmediateContext->UpdateSubresource(m_constantBuffer, 0, NULL, &cb, 0, 0);
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_constantBuffer);

	m_shader->Draw();
	m_pImmediateContext->DrawIndexed(6, 0, 0);

	return true;
}

void MyRender::Close()
{
	_RELEASE(m_vertexBuffer);
	_RELEASE(m_indexBuffer);
	_RELEASE(m_constantBuffer);
	_CLOSE(m_shader);
}


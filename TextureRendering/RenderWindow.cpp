#include "RenderWindow.h"


RenderWindow::RenderWindow()
{
	m_vertexBuffer = nullptr;
	m_indexBuffer = nullptr;
}

bool RenderWindow::Init(ID3D11Device* device, int screenWidth, int screenHeight, int bitmapWidth, int bitmapHeight)
{
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	m_bitmapWidth = bitmapWidth;
	m_bitmapHeight = bitmapHeight;
	m_prevPosX = -1;
	m_prevPosY = -1;

	Vertex vertices[4];

	//Начало коорд. - центр экрана
	signed int centreW = m_screenWidth / 2 * -1;
	signed int centreH = m_screenHeight / 2;
	float left = static_cast<float>(centreW);
	float right = left + m_bitmapWidth;
	float top = static_cast<float>(centreH);
	float bottom = top - m_bitmapHeight;

	vertices[0].pos = XMFLOAT3(left, top, 0.0f);
	vertices[0].tex = XMFLOAT2(0.0f, 0.0f);

	vertices[1].pos = XMFLOAT3(right, bottom, 0.0f);
	vertices[1].tex = XMFLOAT2(1.0f, 1.0f);

	vertices[2].pos = XMFLOAT3(left, bottom, 0.0f);
	vertices[2].tex = XMFLOAT2(0.0f, 1.0f);

	vertices[3].pos = XMFLOAT3(right, top, 0.0f);
	vertices[3].tex = XMFLOAT2(1.0f, 0.0f);

	unsigned long indices[6] = {0,1,2, 0,3,1};

	m_vertexBuffer = Buffer::CreateVertexBuffer(device, sizeof(Vertex)* 4, false, &vertices);
	if (!m_vertexBuffer) return false;

	m_indexBuffer = Buffer::CreateIndexBuffer(device, sizeof(unsigned long)* 6, false, &indices);
	if (!m_indexBuffer) return false;

	return true;
}

void RenderWindow::Close()
{
	_RELEASE(m_indexBuffer);
	_RELEASE(m_vertexBuffer);
}

void RenderWindow::Render(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride = sizeof(Vertex);
	unsigned int offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->DrawIndexed(6, 0, 0);
}
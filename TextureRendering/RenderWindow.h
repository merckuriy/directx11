#pragma once

#include "D3D11_Framework.h"
using namespace D3D11Framework;

//Данный класс используются для отрисовки нашей текстуры на экране.
class RenderWindow
{
	struct Vertex
	{
		XMFLOAT3 pos;
		XMFLOAT2 tex;
	};
public:
	RenderWindow();
	bool Init(ID3D11Device*, int, int, int, int);
	void Close();
	void Render(ID3D11DeviceContext*);

private:
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	int m_screenWidth, m_screenHeight;
	int m_bitmapWidth, m_bitmapHeight;
	int m_prevPosX, m_prevPosY;
};


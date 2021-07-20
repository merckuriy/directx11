#pragma once
#include "D3D11_Framework.h"
#include <xnamath.h>

using namespace D3D11Framework;

class MyRender :public Render
{
public:
	MyRender();
	bool Init(HWND nwnd);
	bool Draw();
	void Close();

	void* operator new(size_t i)
	{
		return _aligned_malloc(i, 16); 
	}

	void operator delete(void* p)
	{
		_aligned_free(p);
	}

private:
	ID3D11Buffer *m_pVertexBuffer;
	ID3D11InputLayout *m_pVertexLayout;
	ID3D11VertexShader *m_pVertexShader;
	ID3D11PixelShader *m_pPixelShader;
	ID3D11PixelShader *m_pPixelShaderSolid; //Шейдер для кубов без освещения.

	ID3D11Buffer *m_pIndexBuffer;
	ID3D11Buffer *m_pConstantBuffer;

	XMMATRIX m_World; 
	XMMATRIX m_View;
	XMMATRIX m_Projection;
};

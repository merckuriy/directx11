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
		//Выделяет память с выравниванием (16 байт). Требуют XMMATRIX.
		return _aligned_malloc(i, 16); 
	}

	void operator delete(void* p)
	{
		_aligned_free(p); //Очищает выравненную память.
	}

private:
	HRESULT m_compileshaderfromfile(WCHAR* FileName, LPCSTR EntryPoint,
		LPCSTR ShaderModel, ID3DBlob** ppBlobOut);

	ID3D11Buffer *m_pVertexBuffer;
	ID3D11InputLayout *m_pVertexLayout;
	ID3D11VertexShader *m_pVertexShader;
	ID3D11PixelShader *m_pPixelShader;

	//Трансформирующие матрицы (XMMATRIX(4х4) = 4х XMVECTOR(x,y,z,w)).
	XMMATRIX m_World;
	XMMATRIX m_View;
	XMMATRIX m_Projection;

	//ID3D11Buffer - неструктурированный буфер, обычно хранящий данные о вершинах или индексах.
	ID3D11Buffer *m_pIndexBuffer; //Индексный буфер
	ID3D11Buffer *m_pConstantBuffer; //Константный буфер. Передаётся в шейдер.
};

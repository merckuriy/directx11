#pragma once
#include "D3D11_Framework.h"

using namespace D3D11Framework;

class MyRender :public Render
{
public:
	MyRender();
	bool Init(HWND nwnd);
	bool Draw();
	void Close();

private:
	//Загружает и компилирует шейдеры из файлов HLSL.
	HRESULT m_compileshaderfromfile(WCHAR* FileName, LPCSTR EntryPoint,
		LPCSTR ShaderModel, ID3DBlob** ppBlobOut);

	ID3D11Buffer *m_pVertexBuffer; //Буфер вершин
	ID3D11InputLayout *m_pVertexLayout; //Входной (в GPU) формат вершин
	ID3D11VertexShader *m_pVertexShader; //Вершинный шейдер. 2 стадия конвеера.
	ID3D11PixelShader *m_pPixelShader; //Пиксельный шейдер. 9 стадия конвеера.
};

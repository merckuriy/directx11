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
	//��������� � ����������� ������� �� ������ HLSL.
	HRESULT m_compileshaderfromfile(WCHAR* FileName, LPCSTR EntryPoint,
		LPCSTR ShaderModel, ID3DBlob** ppBlobOut);

	ID3D11Buffer *m_pVertexBuffer; //����� ������
	ID3D11InputLayout *m_pVertexLayout; //������� (� GPU) ������ ������
	ID3D11VertexShader *m_pVertexShader; //��������� ������. 2 ������ ��������.
	ID3D11PixelShader *m_pPixelShader; //���������� ������. 9 ������ ��������.
};

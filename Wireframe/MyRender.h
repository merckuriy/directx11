#pragma once

#include "D3D11_Framework.h"

using namespace D3D11Framework;

class MyRender :public Render
{
	
public:
	MyRender();
	bool Init(HWND hwnd);
	bool Draw();
	void Close();

private:
	StaticMesh *m_mesh;
	XMMATRIX m_view;
	ID3D11RasterizerState* WireFrame;
	ID3D11RasterizerState* Solid;
};
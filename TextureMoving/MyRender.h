#pragma once

#include "D3D11_Framework.h"

using namespace D3D11Framework;

class MyRender : public Render
{
public:
	MyRender();
	bool Init();
	bool Draw();
	void Close();

private:
	ID3D11Buffer *m_vertexBuffer;
	ID3D11Buffer *m_indexBuffer;
	ID3D11Buffer *m_constMatrixBuffer;
	ID3D11Buffer *m_translateBuffer;

	Shader *m_shader;
	XMMATRIX camView;
};


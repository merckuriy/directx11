#pragma once
#include "D3D11_Framework.h"

using namespace D3D11Framework;

class MyRender;

//используется для рендера глубины в текстуру
class DepthShader
{
public:
	DepthShader(MyRender *render);

	bool Init();
	void Close();
	void Render(int index, CXMMATRIX WVP);

private:
	MyRender *m_render;
	Shader *m_shader;
	ID3D11Buffer *m_matrixBuffer;
};


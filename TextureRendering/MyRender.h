#pragma once

#include "D3D11_Framework.h"

using namespace D3D11Framework;

#include "RenderTarget.h";
#include "RenderWindow.h";

class MyRender :public Render
{
public:
	MyRender();
	bool Init();
	bool Draw();
	void Close();

	void RenderTexture();
	void RenderScene();

private:
	
	ID3D11Buffer *m_vertexBuffer;
	ID3D11Buffer *m_indexBuffer;
	ID3D11Buffer *m_constantBuffer;
	Shader *m_shader;
	Shader *m_shaderRT; //ШЎейдер текстуры
	XMMATRIX camView;

	RenderTarget m_rt;
	RenderWindow m_rw;
};
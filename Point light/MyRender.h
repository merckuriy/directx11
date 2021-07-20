#pragma once

#include "D3D11_Framework.h"

using namespace D3D11Framework;

class MyRender :public Render
{
public:
	MyRender();
	bool Init();
	bool Draw();
	void Close();

private:
	ID3D11Buffer* IndexBuffer;
	ID3D11Buffer* VertBuffer;
	ID3D11Buffer* constMatrixBuffer;
	ID3D11Buffer* constLightBuffer;
	XMMATRIX camView; //Видовая матрица.
	Shader *shader;
};
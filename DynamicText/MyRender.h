#pragma once

#include "D3D11_Framework.h"
#include "Fps.h"

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
	XMMATRIX camView;
	Shader *shader;

	BitmapFont* m_font; //обычный шрифт
	Text* text1; //динамический текст
	BitmapFont* m_font2; //курсив
	Text* text2; //статический текст

	FpsClass fps;
};
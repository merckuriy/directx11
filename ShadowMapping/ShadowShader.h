#pragma once
#include "D3D11_Framework.h"
#include "Light.h"

using namespace D3D11Framework;

class MyRender;

//Выводит конечный результат - модель с тенью.
class ShadowShader
{
public:
	ShadowShader(MyRender *render);

	bool Init();
	void Close();
	void Render(int indexCount, CXMMATRIX worldMatrix, CXMMATRIX WVP, 
		CXMMATRIX WVPlight, ID3D11ShaderResourceView* texture, 
		ID3D11ShaderResourceView* depthMapTexture, Light &light );

private:
	MyRender *m_render;
	Shader *m_shader;
	ID3D11SamplerState* m_sampleStateWrap; //Текстура повторяется при w>1
	ID3D11SamplerState* m_sampleStateClamp; //Текстура обрезается.при w>1: w=1
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_lightBuffer; //буфера для цвета света
	ID3D11Buffer* m_lightBuffer2; //буфер для позиции света
};


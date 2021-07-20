#pragma once
#include "D3D11_Framework.h"
#include "Light.h"

using namespace D3D11Framework;

class MyRender;

//������� �������� ��������� - ������ � �����.
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
	ID3D11SamplerState* m_sampleStateWrap; //�������� ����������� ��� w>1
	ID3D11SamplerState* m_sampleStateClamp; //�������� ����������.��� w>1: w=1
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_lightBuffer; //������ ��� ����� �����
	ID3D11Buffer* m_lightBuffer2; //����� ��� ������� �����
};

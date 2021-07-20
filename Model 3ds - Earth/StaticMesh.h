#pragma once
#include "MyRender.h"

class StaticMesh
{
public:
	StaticMesh();

	bool Init(MyRender *render, wchar_t *name);
	void Render(); //Не наследуется от Render через MyRender, т.к. не виртуальный.
	void Close();

private:
	bool m_loadMS3DFile(wchar_t* name);
	bool m_LoadTextures(wchar_t* name);
	bool m_InitShader(wchar_t* namevs, wchar_t* nameps); //Шейдеры

	void m_RenderBuffers();
	void m_SetShaderParameters();
	void m_RenderShader();

	MyRender *m_render;

	ID3D11Buffer *m_vertexBuffer;
	ID3D11Buffer *m_indexBuffer;
	ID3D11VertexShader *m_vertexShader;
	ID3D11PixelShader *m_pixelShader;
	ID3D11InputLayout *m_layout;
	ID3D11Buffer *m_pConstantBuffer;
	ID3D11SamplerState* m_samplerState;
	ID3D11ShaderResourceView *m_texture;

	XMMATRIX m_objMatrix; //Матрица модели
	unsigned short m_indexCount;
	float m_rot;
};
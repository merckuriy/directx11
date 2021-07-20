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
	friend class MyInput;

	//ѕоверхность в виде сетки, над которой расположен billboard
	ID3D11Buffer *m_vb_grid;
	ID3D11Buffer *m_ib_grid;

	ID3D11Buffer *m_vb_billboard;
	ID3D11Buffer *m_ib_billboard;
	ID3D11Buffer *m_constMatrixBuffer;

	Shader *m_shader;

	ID3D11ShaderResourceView* m_texture_grid;
	ID3D11ShaderResourceView* m_texture_billboard;

	Camera m_cam;

	bool m_leftcam;
	bool m_rightcam;
};


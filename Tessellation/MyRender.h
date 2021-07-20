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
	bool LoadShader();

	Shader *m_shader;
	Camera m_cam;
	ID3D11Buffer* m_vb;
	ID3D11Buffer* m_ib;
	
	//4 стади€ конвеера и 2-а€ тессел€ции
	ID3D11Buffer* m_tessellationBuffer; 
	//ЎШейдер каркаса (3 стади€ конвеера и 1-€ тессел€ции)
	ID3D11HullShader* m_hullShader; 
	//ШЎейдер области (5 стади€ конвеера и 3-€ тессел€ции)
	ID3D11Buffer* m_constMatrixBuffer;
	ID3D11DomainShader* m_domainShader; 

	ID3D11RasterizerState* m_rasterState;
};


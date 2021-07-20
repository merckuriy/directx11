#include "DepthShader.h"
#include "MyRender.h"

struct MatrixBufferType
{
	XMMATRIX WVP;
};

DepthShader::DepthShader(MyRender *render)
{
	m_shader = nullptr;
	m_matrixBuffer = nullptr;
	m_render = render;
}

bool DepthShader::Init()
{
	m_shader = new Shader(m_render);
	m_shader->AddInputElementDesc("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	if (!m_shader->CreateShader(L"depth.vs", L"depth.ps")) return false;

	m_matrixBuffer = Buffer::CreateConstantBuffer(m_render->m_pd3dDevice, sizeof(MatrixBufferType), false);

	return true;
}

void DepthShader::Render(int index, CXMMATRIX WVP)
{
	MatrixBufferType cb;
	cb.WVP = XMMatrixTranspose(WVP);
	m_render->m_pImmediateContext->UpdateSubresource(m_matrixBuffer, 0, NULL, &cb, 0, 0);
	m_render->m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);

	m_shader->Draw();
	m_render->m_pImmediateContext->DrawIndexed(index, 0, 0);
}


void DepthShader::Close()
{
	_CLOSE(m_shader);
	_RELEASE(m_matrixBuffer);
}

#include "ShadowShader.h"
#include "MyRender.h"

struct MatrixBufferType
{
	XMMATRIX world;
	XMMATRIX WVP;
	XMMATRIX wvplight;
	XMMATRIX wvplight2;
};

struct LightBufferType
{
	XMFLOAT4 ambientColor;
	XMFLOAT4 diffuseColor;
	XMFLOAT4 ambientColor2;
	XMFLOAT4 diffuseColor2;
};

struct LightBufferType2
{
	XMFLOAT3 lightPosition;
	float pad;
	XMFLOAT3 lightPosition2;
	float pad2;
};

ShadowShader::ShadowShader(MyRender *render)
{
	m_render = render;
	m_shader = nullptr;
	m_sampleStateWrap = nullptr;
	m_sampleStateClamp = nullptr;
	m_matrixBuffer = nullptr;
	m_lightBuffer = nullptr;
	m_lightBuffer2 = nullptr;
}

bool ShadowShader::Init()
{
	m_shader = new Shader(m_render);
	m_shader->AddInputElementDesc("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	m_shader->AddInputElementDesc("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	m_shader->AddInputElementDesc("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
	if (!m_shader->CreateShader(L"shadow.vs", L"shadow.ps")) return false;

	// Создаем sampler state WRAP
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	if (FAILED(m_render->m_pd3dDevice->CreateSamplerState(&samplerDesc, &m_sampleStateWrap)))
		return false;

	// Создаем sampler state CLAMP
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	if (FAILED(m_render->m_pd3dDevice->CreateSamplerState(&samplerDesc, &m_sampleStateClamp)))
		return false;

	m_matrixBuffer = Buffer::CreateConstantBuffer(m_render->m_pd3dDevice, sizeof(MatrixBufferType), false);
	m_lightBuffer = Buffer::CreateConstantBuffer(m_render->m_pd3dDevice, sizeof(LightBufferType), false);
	m_lightBuffer2 = Buffer::CreateConstantBuffer(m_render->m_pd3dDevice, sizeof(LightBufferType2), false);

	return true;
}

void ShadowShader::Render( int indexCount, CXMMATRIX worldMatrix, 
	CXMMATRIX WVP, CXMMATRIX WVPlight, ID3D11ShaderResourceView* texture, 
	ID3D11ShaderResourceView* depthMapTexture, Light &light,
	CXMMATRIX WVPlight2, ID3D11ShaderResourceView* depthMapTexture2, Light &light2 )
{
	MatrixBufferType cb;
	cb.world = XMMatrixTranspose(worldMatrix);
	cb.WVP = XMMatrixTranspose(WVP);
	cb.wvplight = XMMatrixTranspose(WVPlight);
	cb.wvplight2 = XMMatrixTranspose(WVPlight2);

	m_render->m_pImmediateContext->UpdateSubresource(m_matrixBuffer, 0, NULL, &cb, 0, 0);
	m_render->m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);

	LightBufferType2 lb2;
	lb2.lightPosition = light.GetPosition();
	lb2.lightPosition2 = light2.GetPosition();
	m_render->m_pImmediateContext->UpdateSubresource(m_lightBuffer2, 0, NULL, &lb2, 0, 0);
	m_render->m_pImmediateContext->VSSetConstantBuffers(1, 1, &m_lightBuffer2);

	LightBufferType lb;
	lb.ambientColor = light.GetAmbientColor();
	lb.ambientColor2 = light2.GetAmbientColor();
	lb.diffuseColor = light.GetDiffuseColor();
	lb.diffuseColor2 = light2.GetDiffuseColor();
	m_render->m_pImmediateContext->UpdateSubresource(m_lightBuffer, 0, NULL, &lb, 0, 0);
	m_render->m_pImmediateContext->PSSetConstantBuffers(0, 1, &m_lightBuffer);

	// Передаем в шейдер две текстуры
	m_render->m_pImmediateContext->PSSetShaderResources(0, 1, &texture);
	m_render->m_pImmediateContext->PSSetShaderResources(1, 1, &depthMapTexture);
	m_render->m_pImmediateContext->PSSetShaderResources(2, 1, &depthMapTexture2);

	// Передаем в шейдер оба sampler state
	m_render->m_pImmediateContext->PSSetSamplers(0, 1, &m_sampleStateClamp);
	m_render->m_pImmediateContext->PSSetSamplers(1, 1, &m_sampleStateWrap);

	m_shader->Draw();
	m_render->m_pImmediateContext->DrawIndexed(indexCount, 0, 0);
}

void ShadowShader::Close()
{
	_CLOSE(m_shader);
	_RELEASE(m_sampleStateWrap);
	_RELEASE(m_sampleStateClamp);
	_RELEASE(m_matrixBuffer);
	_RELEASE(m_lightBuffer);
	_RELEASE(m_lightBuffer2);
}
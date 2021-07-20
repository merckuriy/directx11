#include "RenderTarget.h"
#include "MyRender.h"

const int SHADOWMAP_WIDTH = 1024;
const int SHADOWMAP_HEIGHT = 1024;

RenderTarget::RenderTarget(MyRender *render)
{
	m_RTTexture = nullptr;
	m_RTV = nullptr;
	m_SRV = nullptr;
	m_DSTexture = nullptr;
	m_DSV = nullptr;
	m_render = render;
}

bool RenderTarget::Init(float screenNear, float screenDepth)
{
	// —начала создаем текстуру, в которую будем выводить shadow map
	// текстура будет использоваться как render target (D3D11_BIND_RENDER_TARGET)
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = SHADOWMAP_WIDTH;
	textureDesc.Height = SHADOWMAP_HEIGHT;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	if(FAILED(m_render->m_pd3dDevice->CreateTexture2D(&textureDesc, NULL, &m_RTTexture)))
		return false;

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	if (FAILED(m_render->m_pd3dDevice->CreateRenderTargetView(m_RTTexture, &renderTargetViewDesc, &m_RTV)))
		return false;

	// создаем shader resource view c ранее созданной текстуры
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	if (FAILED(m_render->m_pd3dDevice->CreateShaderResourceView(m_RTTexture, &shaderResourceViewDesc, &m_SRV)))
		return false;

	// —оздаем текстуру (карту) глубины
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));
	depthBufferDesc.Width = SHADOWMAP_WIDTH;
	depthBufferDesc.Height = SHADOWMAP_HEIGHT;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;
	if (FAILED(m_render->m_pd3dDevice->CreateTexture2D(&depthBufferDesc, NULL, &m_DSTexture)))
		return false;


	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = depthBufferDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	if (FAILED(m_render->m_pd3dDevice->CreateDepthStencilView(m_DSTexture, &depthStencilViewDesc, &m_DSV)))
		return false;

	// ¬ьюпорт по размеру нашей текстуры
	m_viewport.Width = static_cast<float>(SHADOWMAP_WIDTH);
	m_viewport.Height = static_cast<float>(SHADOWMAP_HEIGHT);
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;

	m_projectionMatrix = XMMatrixPerspectiveFovLH((XM_PI / 4.0f), (m_viewport.Width / m_viewport.Height), screenNear, screenDepth);

	return true;
}

void RenderTarget::Close()
{
	_RELEASE(m_DSV);
	_RELEASE(m_DSTexture);
	_RELEASE(m_SRV);
	_RELEASE(m_RTV);
	_RELEASE(m_RTTexture);
}

void RenderTarget::SetRenderTarget()
{
	//Unbind shader before OMSetRenderTargets.
	ID3D11ShaderResourceView * tab[1] = { NULL };
	m_render->m_pImmediateContext->PSSetShaderResources(1, 1, tab);
	m_render->m_pImmediateContext->OMSetRenderTargets(1, &m_RTV, m_DSV);
	m_render->m_pImmediateContext->RSSetViewports(1, &m_viewport);
}

void RenderTarget::ClearRenderTarget(float red, float green, float blue, float alpha)
{
	float color[4] = { red, green, blue, alpha };
	m_render->m_pImmediateContext->ClearRenderTargetView(m_RTV, color);
	m_render->m_pImmediateContext->ClearDepthStencilView(m_DSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

ID3D11ShaderResourceView* RenderTarget::GetShaderResourceView()
{
	return m_SRV;
}

XMMATRIX RenderTarget::GetProjectionMatrix()
{
	return m_projectionMatrix;
}
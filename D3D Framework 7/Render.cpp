#include "stdafx.h"
#include "Render.h"
#include "macros.h"
#include "StaticMesh.h"
#include "Image.h"
#include "BitmapFont.h"
#include "Text.h"
#include "Log.h"

using namespace D3D11Framework;

Render::Render()
{
	m_pd3dDevice = nullptr;
	m_pImmediateContext = nullptr;
	m_pSwapChain = nullptr;
	m_pRenderTargetView = nullptr;
	m_pDepthStencilView = nullptr;
}

Render::~Render(){}

void Render::m_resize()
{
	RECT rc;
	GetClientRect(m_hwnd, &rc);
	m_width = rc.right - rc.left;
	m_height = rc.bottom - rc.top;
}

bool Render::CreateDevice(HWND hwnd)
{
	m_hwnd = hwnd;

	m_resize();

	if(!m_createdevice()){
		Log::Get()->Err("Не удалось создать DirectX Device");
		return false;
	}

	m_renderstate = new RenderState(m_pd3dDevice, m_pImmediateContext);

	if (!m_renderstate->Init()){
		Log::Get()->Err("Не удалось создать render state");
		return false;
	}

	if (!m_createdepthstencil()){
		Log::Get()->Err("Не удалось создать буфер глубины");
		return false;
	}

	m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	D3D11_VIEWPORT vp;
	vp.Width = static_cast<FLOAT>(m_width);
	vp.Height = static_cast<FLOAT>(m_height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_pImmediateContext->RSSetViewports(1, &vp);

	m_initmatrix();

	return Init();
}

bool Render::m_createdevice()
{
	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = m_width;
	sd.BufferDesc.Height = m_height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = m_hwnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pd3dDevice, NULL, &m_pImmediateContext);
	if(FAILED(hr)) return false;

	ID3D11Texture2D* pBackBuffer = NULL;
	hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&pBackBuffer));
	if(FAILED(hr)) return false;

	hr = m_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pRenderTargetView);
	_RELEASE(pBackBuffer);
	if(FAILED(hr)) return false;

	return true;
}

bool Render::m_createdepthstencil()
{
	//Описание текстуры глубины.
	ID3D11Texture2D *m_pDepthStencil = nullptr;
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = m_width;
	descDepth.Height = m_height;
	descDepth.MipLevels = 1; //макс кол-во мип-уровней в текстуре. 1 так же в случае мультисемплинга.
	descDepth.ArraySize = 1; //Число текстур в текстуре.
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	HRESULT hr = m_pd3dDevice->CreateTexture2D(&descDepth, NULL, &m_pDepthStencil);
	if (FAILED(hr)) return false;

	//Описание вида буфера глубины.
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0; //Индекс первого используемого мип-уровня.
	hr = m_pd3dDevice->CreateDepthStencilView(m_pDepthStencil, &descDSV, &m_pDepthStencilView);
	if (FAILED(hr)) return false;

	_RELEASE(m_pDepthStencil);

	return true;
}


void Render::m_initmatrix()
{
	float aspect = static_cast<float>(m_width) / static_cast<float>(m_height);
	m_Projection = XMMatrixPerspectiveFovLH(0.4f*XM_PI, aspect, 1.0f, 1000.0f);
	m_Ortho = XMMatrixOrthographicLH(static_cast<float>(m_width), static_cast<float>(m_height), 0.0f, 1.0f);
}

void Render::BeginFrame()
{
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, ClearColor);
	m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void Render::EndFrame()
{
	m_pSwapChain->Present(0, 0);
}

void Render::TurnZBufferOn(){
	m_renderstate->TurnZBufferOn();
}

void Render::TurnZBufferOff()
{
	m_renderstate->TurnZBufferOff();
}

void Render::TurnOnAlphaBlending()
{
	m_renderstate->TurnOnAlphaBlending();
}

void Render::TurnOffAlphaBlending()
{
	m_renderstate->TurnOffAlphaBlending();
}

void Render::Shutdown()
{
	Close();

	if(m_pImmediateContext) m_pImmediateContext->ClearState();

	_CLOSE(m_renderstate);
	_RELEASE(m_pDepthStencilView);
	_RELEASE(m_pRenderTargetView);
	_RELEASE(m_pSwapChain);
	_RELEASE(m_pImmediateContext);
	_RELEASE(m_pd3dDevice);
}
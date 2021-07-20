#include "stdafx.h"
#include "RenderState.h"
#include "macros.h"
#include "Log.h"

using namespace D3D11Framework;

RenderState::RenderState(ID3D11Device *pd3dDevice, ID3D11DeviceContext *pImmediateContext)
{
	m_pd3dDevice = pd3dDevice;
	m_pImmediateContext = pImmediateContext;

	m_pDepthEnableStencilState = nullptr;
	m_pDepthDisabledStencilState = nullptr;
	m_pAlphaEnableBlendingState = nullptr;
	m_pAlphaDisableBlendingState = nullptr;
	m_pSampleState = nullptr;

	m_depthenable = false;
	m_alphaenable = false;
}

bool RenderState::Init()
{
	if (!m_createdepthstencilstate()){
		Log::Get()->Err("Не удалось создать stencil state"); return false;
	}
	if (!m_createblendingstate()){
		Log::Get()->Err("Не удалось создать blending state"); return false;
	}
	if (!m_createsamplerstate()){
		Log::Get()->Err("Не удалось создать sampler state"); return false;
	}

	TurnZBufferOn();
	TurnOnAlphaBlending();

	m_pImmediateContext->PSSetSamplers(0, 1, &m_pSampleState);
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return true;
}


bool RenderState::m_createdepthstencilstate()
{
	//Состояние буфера глубины.
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = true;
	//Перед сравнением, значение новых и сущ-их данных будет побитово сопоставляться & с этой маской.
	depthStencilDesc.StencilReadMask = 0xFF; //(D3D11_DEFAULT_STENCIL_READ_MASK) (8 бит)
	depthStencilDesc.StencilWriteMask = 0xFF; //(D3D11_DEFAULT_STENCIL_WRITE_MASK)
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP; //В уроке: INCR
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP; //В уроке: DECR
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	HRESULT hr = m_pd3dDevice->CreateDepthStencilState(&depthStencilDesc, &m_pDepthEnableStencilState);
	if (FAILED(hr)) return false;

	depthStencilDesc.DepthEnable = false;
	hr = m_pd3dDevice->CreateDepthStencilState(&depthStencilDesc, &m_pDepthDisabledStencilState);
	if (FAILED(hr)) return false;

	return true;
}

bool RenderState::m_createblendingstate()
{
	//Cостояние смешивания.
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO; //D3D11_BLEND_ZERO
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0f; //D3D11_COLOR_WRITE_ENABLE_ALL
	blendDesc.AlphaToCoverageEnable = false;
	HRESULT hr = m_pd3dDevice->CreateBlendState(&blendDesc, &m_pAlphaEnableBlendingState);
	if (FAILED(hr)) return false;

	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	hr = m_pd3dDevice->CreateBlendState(&blendDesc, &m_pAlphaDisableBlendingState);
	if (FAILED(hr)) return false;

	return true;
}

bool RenderState::m_createsamplerstate()
{
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.MipLODBias = 0.0f; //Смещение mip уровня.
	sampDesc.MaxAnisotropy = 1; //(1-16)
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	//sampDesc.BorderColor[4] //цвет границ, если указан TEXTURE_ADDRESS_BORDER
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	if (FAILED(m_pd3dDevice->CreateSamplerState(&sampDesc, &m_pSampleState))){
		Log::Get()->Err("Не удалось создать sampler state"); return false; }

	return true;
}

void RenderState::TurnZBufferOn(){
	if (!m_depthenable){
		//2 -заполняем буфер трафарета единицами.
		m_pImmediateContext->OMSetDepthStencilState(m_pDepthEnableStencilState, 1);
		m_depthenable = true;
	}
}


void RenderState::TurnZBufferOff()
{
	if (m_depthenable){
		m_pImmediateContext->OMSetDepthStencilState(m_pDepthDisabledStencilState, 1);
		m_depthenable = false;
	}
	
}

void RenderState::TurnOnAlphaBlending()
{
	if(!m_alphaenable){
		float blendFactor[4] = { 0.0f };
		m_pImmediateContext->OMSetBlendState(m_pAlphaEnableBlendingState, blendFactor, 0xffffffff);
		m_alphaenable = true;
	}
}

void RenderState::TurnOffAlphaBlending()
{
	if (m_alphaenable){
		float blendFactor[4] = { 0.0f };
		m_pImmediateContext->OMSetBlendState(m_pAlphaDisableBlendingState, blendFactor, 0xffffffff);
		m_alphaenable = false;
	}
}

void RenderState::Close()
{
	_RELEASE(m_pAlphaEnableBlendingState);
	_RELEASE(m_pAlphaDisableBlendingState);
	_RELEASE(m_pDepthEnableStencilState);
	_RELEASE(m_pDepthDisabledStencilState);
	_RELEASE(m_pSampleState);
}

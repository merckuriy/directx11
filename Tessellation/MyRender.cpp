#include "MyRender.h"

struct Vertex
{
	XMFLOAT3 pos;
	XMFLOAT4 color;
};

struct WVPBuffer
{
	XMMATRIX wvp;
};

struct TessellationBufferType
{
	float tessellationAmount;
	XMFLOAT3 pad;
};

MyRender::MyRender()
{
	m_vb = nullptr;
	m_ib = nullptr;
	m_constMatrixBuffer = nullptr;
	m_tessellationBuffer = nullptr;
	m_hullShader = nullptr;
	m_domainShader = nullptr;
	m_shader = nullptr;
	m_rasterState = nullptr;
}

bool MyRender::Init()
{
	m_cam.SetPos(0.0f, 0.0f, -2.0f);

	Vertex vert[] =
	{
		{ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3( 0.0f,  1.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3( 1.0f, -1.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) }
	};

	unsigned long indices[] = { 0, 1, 2 };

	m_vb = Buffer::CreateVertexBuffer(m_pd3dDevice, sizeof(Vertex)*3, false, &vert);
	m_ib = Buffer::CreateIndexBuffer(m_pd3dDevice, sizeof(unsigned long)*3, false, &indices);
	m_constMatrixBuffer = Buffer::CreateConstantBuffer(m_pd3dDevice, sizeof(WVPBuffer), false);

	m_tessellationBuffer = Buffer::CreateConstantBuffer(m_pd3dDevice, sizeof(TessellationBufferType), false);

	m_shader = new Shader(this);
	m_shader->AddInputElementDesc("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	m_shader->AddInputElementDesc("COLOR", DXGI_FORMAT_R32G32B32A32_FLOAT);
	if (!m_shader->CreateShader(L"shader.vs", L"shader.ps")) return false;

	//Для вывода сетки геометрии
	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK; //Отбросить заднестор. треуг.
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	if (FAILED(m_pd3dDevice->CreateRasterizerState(&rasterDesc, &m_rasterState)))
		return false;

	m_pImmediateContext->RSSetState(m_rasterState);

	return LoadShader();
	

	return true;
}

bool MyRender::LoadShader()
{
	HRESULT result;

	ID3D10Blob *errorMessage = nullptr;
	ID3D10Blob *hullShaderBuffer = nullptr;
	ID3D10Blob *domainShaderBuffer = nullptr;

	// Компиляция hull shader.
	HRESULT hr = D3DCompileFromFile(L"shader.hs", NULL, NULL, "HS", "hs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &hullShaderBuffer, &errorMessage);
	if (FAILED(hr)){ Log::Get()->Err("Не удалось скомпилировать shader.hs"); return false; }
	// Создание hull shader.
	hr = m_pd3dDevice->CreateHullShader(hullShaderBuffer->GetBufferPointer(), hullShaderBuffer->GetBufferSize(), NULL, &m_hullShader);
	if (FAILED(hr)){ Log::Get()->Err("Не удалось создать hull shader"); return false; }

	// Компиляция domain shader.
	hr = D3DCompileFromFile(L"shader.ds", NULL, NULL, "DS", "ds_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &domainShaderBuffer, &errorMessage);
	if (FAILED(hr)){ Log::Get()->Err("Не удалось скомпилировать shader.ds"); return false; }
	// Создание domain shader.
	hr = m_pd3dDevice->CreateDomainShader(domainShaderBuffer->GetBufferPointer(), domainShaderBuffer->GetBufferSize(), NULL, &m_domainShader);
	if (FAILED(hr)){ Log::Get()->Err("Не удалось создать domain shader"); return false; }

	_RELEASE(hullShaderBuffer);
	_RELEASE(domainShaderBuffer);

	return true;
}

bool MyRender::Draw()
{
	m_cam.Render();
	XMMATRIX camView = m_cam.GetViewMatrix();

	unsigned int stride = sizeof(Vertex);
	unsigned int offset = 0;
	//D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST - Interpret the vertex data as a patch list. (for DS)
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	m_pImmediateContext->IASetVertexBuffers(0, 1, &m_vb, &stride, &offset);
	m_pImmediateContext->IASetIndexBuffer(m_ib, DXGI_FORMAT_R32_UINT, 0);

	TessellationBufferType tes;
	// Количество треугольноков на одной грани.
	tes.tessellationAmount = 24.0f; //12.0f
	m_pImmediateContext->UpdateSubresource(m_tessellationBuffer, 0, NULL, &tes, 0, 0);
	m_pImmediateContext->HSSetConstantBuffers(0, 1, &m_tessellationBuffer);
	m_pImmediateContext->HSSetShader(m_hullShader, NULL, 0);
	
	WVPBuffer cb;
	cb.wvp = XMMatrixTranspose(XMMatrixIdentity()*camView*m_Projection);

	m_pImmediateContext->UpdateSubresource(m_constMatrixBuffer, 0, NULL, &cb, 0, 0);
	m_pImmediateContext->DSSetConstantBuffers(0, 1, &m_constMatrixBuffer);
	m_pImmediateContext->DSSetShader(m_domainShader, NULL, 0);
	
	m_shader->Draw();
	m_pImmediateContext->DrawIndexed(3, 0, 0);

	return true;
}

void MyRender::Close()
{
	_RELEASE(m_vb);
	_RELEASE(m_ib);
	_RELEASE(m_constMatrixBuffer);
	_RELEASE(m_tessellationBuffer);
	_RELEASE(m_hullShader);
	_RELEASE(m_domainShader);
	_RELEASE(m_rasterState);

	_CLOSE(m_shader);
}

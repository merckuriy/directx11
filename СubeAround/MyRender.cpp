#include "MyRender.h"

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
};

MyRender::MyRender()
{
	m_pVertexShader = nullptr;
	m_pPixelShader = nullptr;
	m_pVertexLayout = nullptr;
	m_pVertexBuffer = nullptr;

	m_pIndexBuffer = nullptr;
	m_pConstantBuffer = nullptr;
}


bool MyRender::Init(HWND hwnd)
{
	//Вершинный шейдер.
	HRESULT hr = S_OK;
	ID3DBlob* pVSBlob = NULL;
	hr = m_compileshaderfromfile(L"shader.fx", "VS", "vs_4_0", &pVSBlob);
	if(FAILED(hr)){
		Log::Get()->Err("Не удалось скомпилировать \"VS\" shader.fx. Примеч: файл должен быть в папке с программой.");
		return false;
	}

	hr = m_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), NULL, &m_pVertexShader);
	if(FAILED(hr)){
		Log::Get()->Err("Не удалось создать вершинный шейдер");
		_RELEASE(pVSBlob);
		return false;
	}

	//Описание и создание формата.
	D3D11_INPUT_ELEMENT_DESC layout[] = 
	{	
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(layout);

	hr = m_pd3dDevice->CreateInputLayout(layout, numElements,
		pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexLayout);
	_RELEASE(pVSBlob);
	if(FAILED(hr)) return false;

	m_pImmediateContext->IASetInputLayout(m_pVertexLayout);

	//Пиксельный шейдер.
	ID3DBlob* pPSBlob = NULL;
	hr = m_compileshaderfromfile(L"shader.fx", "PS", "ps_4_0", &pPSBlob);
	if(FAILED(hr)){
		Log::Get()->Err("Не удалось скомпилировать \"PS\" shader.fx. Примеч: файл должен быть в папке с программой.");
		return false;
	}

	hr = m_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),
		pPSBlob->GetBufferSize(), NULL, &m_pPixelShader);
	_RELEASE(pPSBlob);
	if(FAILED(hr)){
		Log::Get()->Err("Не удалось создать пиксельный шейдер");
		return false;
	}

	//Создание буфера вершин.
	SimpleVertex vertices[] =
	{
		/*0*/{ XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
		/*1*/{ XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		/*2*/{ XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
		/*3*/{ XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		/*4*/{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
		/*5*/{ XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		/*6*/{ XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		/*7*/{ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) }
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT; 
	bd.ByteWidth = sizeof(SimpleVertex)*8;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;

	hr = m_pd3dDevice->CreateBuffer(&bd, &InitData, &m_pVertexBuffer);
	if(FAILED(hr)) return false;

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	m_pImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//Индексный буфер.  
	WORD indices[] = 
	{
		3,1,0,	2,1,3, 
		0,5,4,	1,5,0,
		3,4,7,	0,4,3,
		1,6,5,	2,6,1,
		2,7,6,	3,7,2,
		6,4,5,	7,4,6
	};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD)*36;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = indices;
	hr = m_pd3dDevice->CreateBuffer(&bd, &InitData, &m_pIndexBuffer);
	if(FAILED(hr)) return false;

	m_pImmediateContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Константный буфер.
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = m_pd3dDevice->CreateBuffer(&bd, NULL, &m_pConstantBuffer);
	if(FAILED(hr)) return false;

	m_World1 = XMMatrixIdentity();
	m_World2 = XMMatrixIdentity();
	m_World3 = XMMatrixIdentity();

	XMVECTOR Eye = XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 0.5f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_View = XMMatrixLookAtLH(Eye, At, Up);

	float width = 640.0f;
	float height = 480.0f;
	m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, width/height, 0.01f, 100.0f);

	return true;
}

void MyRender::Update()
{
	static float t = 0.0f;
	static DWORD dwTimeStart = 0;
	DWORD dwTimeCur = GetTickCount();
	if(dwTimeStart == 0) dwTimeStart = dwTimeCur;
	t = (dwTimeCur - dwTimeStart) / 1000.0f;

	m_World1 = XMMatrixRotationY(t)*XMMatrixRotationX(t);

	XMMATRIX mScale = XMMatrixScaling(0.3f, 0.3f, 0.3f); //1.Масштаб.
	XMMATRIX mSpin = XMMatrixRotationZ(-t); //2.Вращение вокруг своей оси. (против часовой)
	XMMATRIX mTranslate = XMMatrixTranslation(-2.0f, 0.0f, 0.0f); //3.Перемещение.
	XMMATRIX mOrbit = XMMatrixRotationY(-t*3.0f)*XMMatrixRotationX(t); //4.Вращение всего объекта вокруг центра.

	//Порядок важен, т.к. трансформации применяются к началу координат.
	m_World2 = mScale * mSpin * mTranslate * mOrbit; 

	mScale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
	mSpin = XMMatrixRotationZ(t); 
	mTranslate = XMMatrixTranslation(-3.0f, 0.0f, 0.0f); 
	mOrbit = XMMatrixRotationX(-t*8.0f)*XMMatrixRotationY(-t*6.0f)*XMMatrixRotationZ(-t);

	m_World3 = mScale * mSpin * mTranslate * mOrbit;
}


bool MyRender::Draw()
{
	Update();
	m_pImmediateContext->VSSetShader(m_pVertexShader, NULL, 0); //2 стадия
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer); //2 стадия
	m_pImmediateContext->PSSetShader(m_pPixelShader, NULL, 0); //9 стадия

	ConstantBuffer cb1;
	cb1.mWorld = XMMatrixTranspose(m_World1);
	cb1.mView = XMMatrixTranspose(m_View);
	cb1.mProjection = XMMatrixTranspose(m_Projection);
	m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, NULL, &cb1, 0, 0);
	m_pImmediateContext->DrawIndexed(36, 0, 0);

	ConstantBuffer cb2;
	cb2.mWorld = XMMatrixTranspose(m_World2);
	cb2.mView = XMMatrixTranspose(m_View);
	cb2.mProjection = XMMatrixTranspose(m_Projection);
	m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, NULL, &cb2, 0, 0);
	m_pImmediateContext->DrawIndexed(36, 0, 0);

	cb2.mWorld = XMMatrixTranspose(m_World3);
	cb2.mView = XMMatrixTranspose(m_View);
	cb2.mProjection = XMMatrixTranspose(m_Projection);
	m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, NULL, &cb2, 0, 0);
	m_pImmediateContext->DrawIndexed(36, 0, 0);

	return true;
}

void MyRender::Close()
{
	_RELEASE(m_pConstantBuffer);
	_RELEASE(m_pVertexBuffer);
	_RELEASE(m_pIndexBuffer);
	_RELEASE(m_pVertexLayout);
	_RELEASE(m_pVertexShader);
	_RELEASE(m_pPixelShader);
}
#include "MyRender.h"

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
};

struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMFLOAT4 vLightDir[2];
	XMFLOAT4 vLightColor[2];
	XMFLOAT4 vOutputColor;
};

MyRender::MyRender()
{
	m_pVertexShader = nullptr;
	m_pPixelShader = nullptr;
	m_pPixelShaderSolid = nullptr;
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
	if(FAILED(hr)){return false;}

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
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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
	if(FAILED(hr)){return false;}

	hr = m_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),
		pPSBlob->GetBufferSize(), NULL, &m_pPixelShader);
	_RELEASE(pPSBlob);
	if(FAILED(hr)){
		Log::Get()->Err("Не удалось создать пиксельный шейдер");
		return false;
	}

	pPSBlob = NULL;
	hr = m_compileshaderfromfile(L"shader.fx", "PSSolid", "ps_4_0", &pPSBlob);
	if(FAILED(hr)){return false;}

	hr = m_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),
		pPSBlob->GetBufferSize(), NULL, &m_pPixelShaderSolid);
	_RELEASE(pPSBlob);
	if(FAILED(hr)){
		Log::Get()->Err("Не удалось создать пиксельный шейдер");
		return false;
	}

	//Создание буфера вершин. Всё просто. Создаём каждую грань через одни вершины, но с разными нормалями.
	SimpleVertex vertices[] =
	{
		//Смотрим на нормаль: y=1, значит верхняя грань.
		{ XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3( 0.0f,  1.0f,  0.0f) },
		{ XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT3( 0.0f,  1.0f,  0.0f) },
		{ XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT3( 0.0f,  1.0f,  0.0f) },
		{ XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3( 0.0f,  1.0f,  0.0f) },
		
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3( 0.0f, -1.0f,  0.0f) },
		{ XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT3( 0.0f, -1.0f,  0.0f) },
		{ XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT3( 0.0f, -1.0f,  0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3( 0.0f, -1.0f,  0.0f) },

		{ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(-1.0f,  0.0f,  0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f,  0.0f,  0.0f) },
		{ XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(-1.0f,  0.0f,  0.0f) },
		{ XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(-1.0f,  0.0f,  0.0f) },

		{ XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT3( 1.0f,  0.0f,  0.0f) },
		{ XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT3( 1.0f,  0.0f,  0.0f) },
		{ XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT3( 1.0f,  0.0f,  0.0f) },
		{ XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT3( 1.0f,  0.0f,  0.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3( 0.0f,  0.0f, -1.0f) },
		{ XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT3( 0.0f,  0.0f, -1.0f) },
		{ XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT3( 0.0f,  0.0f, -1.0f) },
		{ XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3( 0.0f,  0.0f, -1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3( 0.0f,  0.0f,  1.0f) },
		{ XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT3( 0.0f,  0.0f,  1.0f) },
		{ XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT3( 0.0f,  0.0f,  1.0f) },
		{ XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3( 0.0f,  0.0f,  1.0f) },
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT; 
	bd.ByteWidth = sizeof(SimpleVertex)*24;
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

		6,4,5,	7,4,6,

		11,9,8,	10,9,11,

		14,12,13,	15,12,14,

		19,17,16,	18,17,19,

		22,20,21,	23,20,22
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

	m_World = XMMatrixIdentity();

	XMVECTOR Eye = XMVectorSet(0.0f, 3.0f, -7.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_View = XMMatrixLookAtLH(Eye, At, Up);

	float width = 640.0f;
	float height = 480.0f;
	m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, width/height, 0.01f, 100.0f);

	return true;
}

bool MyRender::Draw()
{
	static float t = 0.0f;
	static DWORD dwTimeStart = 0;
	DWORD dwTimeCur = GetTickCount();
	if(dwTimeStart == 0) dwTimeStart = dwTimeCur;
	t = (dwTimeCur - dwTimeStart) / 1000.0f;

	m_World = XMMatrixRotationY(t);

	//Расположение направленных источников света (направление на начало координат).
	XMFLOAT4 vLightDirs[2] =
	{
		XMFLOAT4(-0.577f, 0.577f, -0.577f, 1.0f), //сверху
		XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f) //сбоку
	};

	XMFLOAT4 vLightColors[2] =
	{
		XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), //синий, стационарный
		XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)  //красный, вращается (точнее меняет направление)
	};

	//Вращаем ветор красного цвета.
	XMMATRIX mRotate = XMMatrixRotationY(-2.0f*t);
	//Преобразует XMFLOAT4 в XMVECTOR. XMVECTOR является аппаратно-зависимым.
	XMVECTOR vLightDir = XMLoadFloat4(&vLightDirs[1]);
	//Умножение вектора на матрицу (трансформация вектора).
	vLightDir = XMVector3Transform(vLightDir, mRotate); 
	//Сохраняет XMVECTOR в XMFLOAT4.
	XMStoreFloat4(&vLightDirs[1], vLightDir);

	static float z = vLightDirs[0].z;
	static bool forward = true; //направление
	if(z > 1.0f || z < -1.0f) forward = !forward;
	if(forward) z += 0.0001f; else z -= 0.0001f;

	//Вращаем ветор синего цвета.
	mRotate = XMMatrixTranslation(0.0f,0.0f,z);
	//Преобразует XMFLOAT4 в XMVECTOR. XMVECTOR является аппаратно-зависимым.
	vLightDir = XMLoadFloat4(&vLightDirs[0]);
	//Умножение вектора на матрицу (трансформация вектора).
	vLightDir = XMVector3Transform(vLightDir, mRotate);
	//Сохраняет XMVECTOR в XMFLOAT4.
	XMStoreFloat4(&vLightDirs[0], vLightDir);

	ConstantBuffer cb1;
	cb1.mWorld = XMMatrixTranspose(m_World);
	cb1.mView = XMMatrixTranspose(m_View);
	cb1.mProjection = XMMatrixTranspose(m_Projection);
	cb1.vLightDir[0] = vLightDirs[0];
	cb1.vLightDir[1] = vLightDirs[1];
	cb1.vLightColor[0] = vLightColors[0];
	cb1.vLightColor[1] = vLightColors[1];
	cb1.vOutputColor = XMFLOAT4(0, 0, 0, 0); //Цвет неосвещенного куба
	m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, NULL, &cb1, 0, 0);
	
	m_pImmediateContext->VSSetShader(m_pVertexShader, NULL, 0); //2 стадия
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer); //2 стадия
	m_pImmediateContext->PSSetShader(m_pPixelShader, NULL, 0); //9 стадия
	m_pImmediateContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer); //9 стадия
	m_pImmediateContext->DrawIndexed(36, 0, 0); //Рисуем 1-ый куб.
	
	//Рисуем два куба, показывающих направление и цвет источников света.
	for(int m = 0; m < 2; m++){
		//Трансляционная матрица кубов на основе вектора смещения (от расположения источников света).
		XMMATRIX mLight = XMMatrixTranslationFromVector(5.0f*XMLoadFloat4(&vLightDirs[m]));
		XMMATRIX mLightScale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
		mLight = mLightScale * mLight; //как мировая матрица.

		cb1.mWorld = XMMatrixTranspose(mLight);
		cb1.vOutputColor = vLightColors[m];
		m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, NULL, &cb1, 0, 0);

		//Устанавливаем другой шейдер, чтобы освещение не влияло на данные кубики.
		m_pImmediateContext->PSSetShader(m_pPixelShaderSolid, NULL, 0);
		m_pImmediateContext->DrawIndexed(36, 0, 0);
	}

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
	_RELEASE(m_pPixelShaderSolid);
}
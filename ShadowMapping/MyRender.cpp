#include "MyRender.h"

struct Vertex
{
	Vertex(float x, float y, float z, float u, float v, float nx, float ny, float nz) : 
		position(x, y, z), texture(u, v), normal(nx, ny, nz){}

	XMFLOAT3 position;
	XMFLOAT2 texture;
	XMFLOAT3 normal;
};

MyRender::MyRender()
{
	m_key_up = false;
	m_key_down = false;
	m_key_left = false;
	m_key_right = false;
	m_key_a = false;
	m_key_z = false;
	m_key_s = false;
	m_key_x = false;
}

bool MyRender::Init()
{
	//Во фреймворке нет метода получения вьюпорта, поэтому придётся его продублировать.
	m_viewport.Width = (float)m_width;
	m_viewport.Height = (float)m_height;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;
	m_pImmediateContext->RSSetViewports(1, &m_viewport);

	m_cam.SetPos(0.0f, 4.0f, -10.0f);

	m_posbox1 = XMFLOAT3(-1.0f, 2.0f, 0.0f);
	m_posbox2 = XMFLOAT3(2.0f, 2.0f, 0.0f);

	// настраиваем свет
	m_Light.SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
	m_Light.SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light.SetLookAt(0.0f, 0.0f, 0.0f);
	m_Light.GenerateProjectionMatrix(1.0f, 100.0f);

	m_RenderTexture = new RenderTarget(this);
	if (!m_RenderTexture->Init(1.0f, 100.0f)){
		Log::Get()->Err("Ошибка инициализации Render Texture"); return false;
	}
	m_DepthShader = new DepthShader(this);
	if (!m_DepthShader->Init()){
		Log::Get()->Err("Ошибка инициализации Depth Shader"); return false;
	}
	m_ShadowShader = new ShadowShader(this);
	if (!m_ShadowShader->Init()){
		Log::Get()->Err("Ошибка инициализации Shadow Shader"); return false;
	}

	// геометрия (plane)
	Vertex vert1[] =
	{
		Vertex(-5.0f, 0.0f,  5.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f),
		Vertex( 5.0f, 0.0f,  5.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f),
		Vertex(-5.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f),
		Vertex(-5.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f),
		Vertex( 5.0f, 0.0f,  5.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f),
		Vertex( 5.0f, 0.0f, -5.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f)
	};
	unsigned long indices1[6] ={ 0,1,2, 3,4,5 };

	m_vb_ground = Buffer::CreateVertexBuffer(m_pd3dDevice, sizeof(Vertex)* 6, false, &vert1);
	m_ib_ground = Buffer::CreateIndexBuffer(m_pd3dDevice, sizeof(unsigned long)* 6, false, &indices1);
	D3DX11CreateShaderResourceViewFromFile(m_pd3dDevice, L"stone01.jpg", NULL, NULL, &m_texture_ground, NULL);

	// геометрия (box)
	Vertex vert2[] =
	{
		Vertex(-1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f),
		Vertex(-1.0f,  1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f),
		Vertex( 1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f),
		Vertex( 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f),

		Vertex(-1.0f, -1.0f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f, 1.0f),
		Vertex( 1.0f, -1.0f,  1.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f),
		Vertex( 1.0f,  1.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f),
		Vertex(-1.0f,  1.0f,  1.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f),

		Vertex(-1.0f,  1.0f, -1.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f),
		Vertex(-1.0f,  1.0f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f),
		Vertex( 1.0f,  1.0f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f),
		Vertex( 1.0f,  1.0f, -1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f),

		Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f,  0.0f, -1.0f, 0.0f),
		Vertex( 1.0f, -1.0f, -1.0f, 0.0f, 1.0f,  0.0f, -1.0f, 0.0f),
		Vertex( 1.0f, -1.0f,  1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f),
		Vertex(-1.0f, -1.0f,  1.0f, 1.0f, 0.0f,  0.0f, -1.0f, 0.0f),

		Vertex(-1.0f, -1.0f,  1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f),
		Vertex(-1.0f,  1.0f,  1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		Vertex(-1.0f,  1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f),

		Vertex( 1.0f, -1.0f, -1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 0.0f),
		Vertex( 1.0f,  1.0f, -1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f),
		Vertex( 1.0f,  1.0f,  1.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f),
		Vertex( 1.0f, -1.0f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f, 0.0f),
	};

	unsigned long indices2[] =
	{
		0,1,2, 0,2,3,
		4,5,6, 4,6,7,
		8,9,10, 8,10,11,
		12,13,14, 12,14,15,
		16,17,18, 16,18,19,
		20,21,22, 20,22,23
	};

	m_vb_box = Buffer::CreateVertexBuffer(m_pd3dDevice, sizeof(Vertex)* 24, false, vert2);
	m_ib_box = Buffer::CreateIndexBuffer(m_pd3dDevice, sizeof(unsigned long)* 36, false, indices2);
	// грузим текстуры для кубов
	D3DX11CreateShaderResourceViewFromFile(m_pd3dDevice, L"wall01.jpg", NULL, NULL, &m_texture_box1, NULL);
	D3DX11CreateShaderResourceViewFromFile(m_pd3dDevice, L"ice.jpg", NULL, NULL, &m_texture_box2, NULL);

	return true;
}

bool MyRender::Draw()
{
	static float lightPositionX = -5.0f;
	static bool reverse = false;
	// изменяем позицию света по X
	if (!reverse) lightPositionX += 0.001f;
	else lightPositionX -= 0.001f;

	if (lightPositionX > 5.0f) reverse = true;
	else if (lightPositionX < -5.0f) reverse = false;
	m_Light.SetPosition(lightPositionX, 8.0f, -5.0f);
	
	// Генерируем видовую матрицу для света
	m_Light.GenerateViewMatrix();

	// управление камерой
	m_cam.MoveForward(m_key_up);
	m_cam.MoveBackward(m_key_down);
	m_cam.RotateLeft(m_key_left);
	m_cam.RotateRight(m_key_right);
	m_cam.MoveUpward(m_key_a);
	m_cam.MoveDownward(m_key_z);
	m_cam.LookUpward(m_key_s);
	m_cam.LookDownward(m_key_x);

	m_cam.Render();

	// рендерим сцену в текстуру (получаем текстуру глубины (дистанции))
	RenderSceneToTexture();

	// рендерим сцену на экран
	RenderSceneToWindow(); //закомментить для карты тени
		
	return true;
}

void MyRender::RenderSceneToTexture()
{
	XMMATRIX WVP;
	unsigned int stride = sizeof(Vertex);
	unsigned int offset = 0;

	//Указываем что нужно рендерить в текстуру
	m_RenderTexture->SetRenderTarget(); //закомментить для карты тени
	m_RenderTexture->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);

	// Выводим первый ящик
	XMMATRIX wldMatrix = XMMatrixTranslation(m_posbox1.x, m_posbox1.y, m_posbox1.z);
	WVP = wldMatrix* m_Light.GetViewMatrix()* m_Light.GetProjectionMatrix();
	m_pImmediateContext->IASetVertexBuffers(0, 1, &m_vb_box, &stride, &offset);
	m_pImmediateContext->IASetIndexBuffer(m_ib_box, DXGI_FORMAT_R32_UINT, 0);
	m_DepthShader->Render(36, WVP);

	// Выводим второй ящик
	wldMatrix = XMMatrixTranslation(m_posbox2.x, m_posbox2.y, m_posbox2.z);
	WVP = wldMatrix* m_Light.GetViewMatrix()* m_Light.GetProjectionMatrix();
	m_pImmediateContext->IASetVertexBuffers(0, 1, &m_vb_box, &stride, &offset);
	m_pImmediateContext->IASetIndexBuffer(m_ib_box, DXGI_FORMAT_R32_UINT, 0);
	m_DepthShader->Render(36, WVP);

	// Выводим поверхность
	wldMatrix = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	WVP = wldMatrix* m_Light.GetViewMatrix()* m_Light.GetProjectionMatrix();
	m_pImmediateContext->IASetVertexBuffers(0, 1, &m_vb_ground, &stride, &offset);
	m_pImmediateContext->IASetIndexBuffer(m_ib_ground, DXGI_FORMAT_R32_UINT, 0);
	m_DepthShader->Render(6, WVP);
}

void MyRender::RenderSceneToWindow()
{
	// Ставим render target экрана
	m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);
	// Ставим вьюпорт экрана
	m_pImmediateContext->RSSetViewports(1, &m_viewport);

	XMMATRIX camView = m_cam.GetViewMatrix();
	XMMATRIX lightViewMatrix = m_Light.GetViewMatrix();
	XMMATRIX lightProjectionMatrix = m_Light.GetProjectionMatrix();

	unsigned int stride = sizeof(Vertex);
	unsigned int offset = 0;
	XMMATRIX wvp;
	XMMATRIX wvplight;
	XMMATRIX wldMatrix;

	// Выводим первый куб
	wldMatrix = XMMatrixTranslation(m_posbox1.x, m_posbox1.y, m_posbox1.z);
	wvp = wldMatrix * camView * m_Projection;
	wvplight = wldMatrix * lightViewMatrix * lightProjectionMatrix;
	m_pImmediateContext->IASetVertexBuffers(0, 1, &m_vb_box, &stride, &offset);
	m_pImmediateContext->IASetIndexBuffer(m_ib_box, DXGI_FORMAT_R32_UINT, 0);
	m_ShadowShader->Render(36, wldMatrix, wvp, wvplight, m_texture_box1, m_RenderTexture->GetShaderResourceView(), m_Light);

	// Выводим второй куб
	wldMatrix = XMMatrixTranslation(m_posbox2.x, m_posbox2.y, m_posbox2.z);
	wvp = wldMatrix * camView * m_Projection;
	wvplight = wldMatrix * lightViewMatrix * lightProjectionMatrix;
	m_pImmediateContext->IASetVertexBuffers(0, 1, &m_vb_box, &stride, &offset);
	m_pImmediateContext->IASetIndexBuffer(m_ib_box, DXGI_FORMAT_R32_UINT, 0);
	m_ShadowShader->Render(36, wldMatrix, wvp, wvplight, m_texture_box2, m_RenderTexture->GetShaderResourceView(), m_Light);

	// выводим поверхность
	wldMatrix = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	wvp = wldMatrix * camView * m_Projection;
	wvplight = wldMatrix * lightViewMatrix * lightProjectionMatrix;
	m_pImmediateContext->IASetVertexBuffers(0, 1, &m_vb_ground, &stride, &offset);
	m_pImmediateContext->IASetIndexBuffer(m_ib_ground, DXGI_FORMAT_R32_UINT, 0);
	m_ShadowShader->Render(6, wldMatrix, wvp, wvplight, m_texture_ground, m_RenderTexture->GetShaderResourceView(), m_Light);

}

void MyRender::Close()
{
	_CLOSE(m_RenderTexture);
	_CLOSE(m_DepthShader);
	_CLOSE(m_ShadowShader);

	_RELEASE(m_vb_ground);
	_RELEASE(m_ib_ground);
	_RELEASE(m_vb_box);
	_RELEASE(m_ib_box);
	_RELEASE(m_texture_ground);
	_RELEASE(m_texture_box1);
	_RELEASE(m_texture_box2);
}
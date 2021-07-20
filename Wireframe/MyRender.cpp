#include "MyRender.h"

MyRender::MyRender(){ 
	m_mesh = nullptr;
	WireFrame = nullptr;
	Solid = nullptr;
}

bool MyRender::Init(HWND hwnd)
{
	XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, -2.8f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_view = XMMatrixLookAtLH(Eye, At, Up);

	m_mesh = new StaticMesh();
	if(!m_mesh->Init(this, L"mesh.ms3d"))
		return false;

	//см. ниже
	D3D11_RASTERIZER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_RASTERIZER_DESC));
	desc.FillMode = D3D11_FILL_WIREFRAME; //сетка
	desc.CullMode = D3D11_CULL_NONE; //–исовать и передние и задние треугольники.
	m_pd3dDevice->CreateRasterizerState(&desc, &WireFrame);
	desc.FillMode = D3D11_FILL_SOLID;
	m_pd3dDevice->CreateRasterizerState(&desc, &Solid);

	return true;
}

bool MyRender::Draw()
{
	static float rot = 0.0f;
	rot += .0005f;
	if(rot > 6.26f)
		rot = 0.0f;

	//ѕривязываем к стадии растеризации RS.
	m_pImmediateContext->RSSetState(Solid);
	m_mesh->Identity();
	m_mesh->Rotate(-rot, 0.0, 1.0, 0.0);
	m_mesh->Translate(-1.5, 0.0, 0.0);
	m_mesh->Draw(m_view);

	m_pImmediateContext->RSSetState(WireFrame);
	m_mesh->Identity();
	m_mesh->Rotate(rot, 0.0, 1.0, 0.0);
	m_mesh->Translate(1.5, 0.0, 0.0);
	m_mesh->Draw(m_view);
	return true;
}

void MyRender::Close()
{
	_CLOSE(m_mesh);
	_RELEASE(WireFrame);
	_RELEASE(Solid);
}

/* ќписывает состояние растеризации.
typedef struct D3D11_RASTERIZER_DESC {
D3D11_FILL_MODE FillMode; //сетка или сплошная поверхность.
D3D11_CULL_MODE CullMode; //рисовать ли только лицевую или заднюю поверхность.
BOOL    FrontCounterClockwise; //Ћицевая сторона, если против часовой.
INT     DepthBias; //глубина смещения.
FLOAT   DepthBiasClamp;	//max глубина смещения.
FLOAT   SlopeScaledDepthBias; //велечина уклона.
BOOL    DepthClipEnable;//вкл. обрезку глубины. (т.е. 0<=z<=w)
BOOL    ScissorEnable;	//ѕрорисовка вне области отсечения.
BOOL    MultisampleEnable; //вкл. quadrilateral line антиалиасинг 
						   //вместо alpha la. ѕри MSAA.
BOOL    AntialiasedLineEnable; //вкл. line antialiasing. ѕри рисовании линий.
} D3D11_RASTERIZER_DESC;
*/
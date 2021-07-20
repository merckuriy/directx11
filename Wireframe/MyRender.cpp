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

	//��. ����
	D3D11_RASTERIZER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_RASTERIZER_DESC));
	desc.FillMode = D3D11_FILL_WIREFRAME; //�����
	desc.CullMode = D3D11_CULL_NONE; //�������� � �������� � ������ ������������.
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

	//����������� � ������ ������������ RS.
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

/* ��������� ��������� ������������.
typedef struct D3D11_RASTERIZER_DESC {
D3D11_FILL_MODE FillMode; //����� ��� �������� �����������.
D3D11_CULL_MODE CullMode; //�������� �� ������ ������� ��� ������ �����������.
BOOL    FrontCounterClockwise; //������� �������, ���� ������ �������.
INT     DepthBias; //������� ��������.
FLOAT   DepthBiasClamp;	//max ������� ��������.
FLOAT   SlopeScaledDepthBias; //�������� ������.
BOOL    DepthClipEnable;//���. ������� �������. (�.�. 0<=z<=w)
BOOL    ScissorEnable;	//���������� ��� ������� ���������.
BOOL    MultisampleEnable; //���. quadrilateral line ������������ 
						   //������ alpha la. ��� MSAA.
BOOL    AntialiasedLineEnable; //���. line antialiasing. ��� ��������� �����.
} D3D11_RASTERIZER_DESC;
*/
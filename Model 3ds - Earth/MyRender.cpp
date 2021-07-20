#include "MyRender.h"
#include "StaticMesh.h"

MyRender::MyRender()
{
	m_mesh = nullptr;
}


bool MyRender::Init(HWND hwnd)
{
	XMVECTOR Eye = XMVectorSet(0.0f, 1.0f, -12.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 0.2f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_View = XMMatrixLookAtLH(Eye, At, Up);

	m_Projection = XMMatrixPerspectiveFovLH(0.4f*3.14f, 640.0f / 480.0f, 0.1f, 1000.0f); //(0.4*XM_PI) XM_PIDIV2

	m_mesh = new StaticMesh();
	if(!m_mesh->Init(this, L"Earth.3ds")) return false; //TronGun Sphere Earth

	return true;
}


bool MyRender::Draw()
{
	m_mesh->Render();
	return true;
}

void MyRender::Close()
{
	_CLOSE(m_mesh);
}
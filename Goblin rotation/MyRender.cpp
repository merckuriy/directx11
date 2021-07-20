#include "MyRender.h"

MyRender::MyRender(){ m_mesh = nullptr; }

bool MyRender::Init(HWND hwnd)
{
	XMVECTOR Eye = XMVectorSet(0.0f, 2.5f, -3.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	/*XMVECTOR Eye = XMVectorSet(0.0f, 4.5f, 0.01f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);*/

	m_View = XMMatrixLookAtLH(Eye, At, Up);

	m_mesh = new StaticMesh();
	if(!m_mesh->Init(this, L"mesh.ms3d")) return false;

	return true;
}

bool MyRender::Draw()
{
	static float rot = 0.0f;
	static float acceleration = 0.0f;
	acceleration += .00000004f;
	rot = rot + .0005f + acceleration;
	//rot *= 1.0001f; //rot = 0.1f;
	if(rot > XM_2PI) rot = 0.0f; //6.26f

	m_mesh->Identity();
	m_mesh->Rotate(-rot, 0.0, 1.0, 0.0);
	//m_mesh->Translate(1.5, 0.0, 0.0);
	m_mesh->Draw(m_View);

	m_mesh->Identity();	
	m_mesh->Translate(-1.5, 0.0, 0.0);
	m_mesh->Rotate(rot, 0.0, 1.0, 0.0);
	m_mesh->Draw(m_View);
	return true;
}

void MyRender::Close()
{
	_CLOSE(m_mesh);
}
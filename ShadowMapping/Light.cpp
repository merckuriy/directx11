#include "Light.h"

void Light::SetAmbientColor(float red, float green, float blue, float alpha)
{
	m_ambientColor = XMFLOAT4(red, green, blue, alpha);
}

void Light::SetDiffuseColor(float red, float green, float blue, float alpha)
{
	m_diffuseColor = XMFLOAT4(red, green, blue, alpha);
}

void Light::SetPosition(float x, float y, float z)
{
	m_position = XMFLOAT3(x, y, z);
}

void Light::SetLookAt(float x, float y, float z)
{
	m_lookAt = XMFLOAT3(x, y, z);
}

XMFLOAT4 Light::GetAmbientColor()
{
	return m_ambientColor;
}

XMFLOAT4 Light::GetDiffuseColor()
{
	return m_diffuseColor;
}

XMFLOAT3 Light::GetPosition()
{
	return m_position;
}

void Light::GenerateViewMatrix()
{
	XMVECTOR camPos = XMVectorSet(m_position.x, m_position.y, m_position.z, 0.0f);
	XMVECTOR camLookAt = XMVectorSet(m_lookAt.x, m_lookAt.y, m_lookAt.z, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	m_viewMatrix = XMMatrixLookAtLH(camPos, camLookAt, Up);
}

void Light::GenerateProjectionMatrix(float screenNear, float screenDepth)
{
	float fieldOfView = XM_PI / 2.0f;
	float screenAspect = 1.0f; //квадрат
	m_projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);
}

XMMATRIX Light::GetViewMatrix()
{
	return m_viewMatrix;
}

XMMATRIX Light::GetProjectionMatrix()
{
	return m_projectionMatrix;
}

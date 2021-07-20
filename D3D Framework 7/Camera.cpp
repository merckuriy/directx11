#include "stdafx.h"
#include "Camera.h"

using namespace D3D11Framework;

Camera::Camera()
{
	m_frameTime = 0.0f;

	m_leftSpeed = 0.0f;
	m_rightSpeed = 0.0f;
	m_forwardSpeed = 0.0f;
	m_backwardSpeed = 0.0f;
	m_upwardSpeed = 0.0f;
	m_downwardSpeed = 0.0f;
	m_leftTurnSpeed = 0.0f;
	m_rightTurnSpeed = 0.0f;
	m_lookUpSpeed = 0.0f;
	m_lookDownSpeed = 0.0f;

	m_pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_rot = XMFLOAT3(0.0f, 0.0f, 0.0f);

	m_time.Init();
}

void Camera::Render()
{
	m_time.Frame();
	m_frameTime = m_time.GetTime();

	//0.0174532925 = PI/180 (шаг - 1 градус)
	float pitch = m_rot.x * 0.0174532925f; //взгляд вверх/вниз
	float yaw = m_rot.y * 0.0174532925f; //взгляд влево/вправо
	float roll = m_rot.z * 0.0174532925f; //переворот взгляда (вращение)

	XMMATRIX rotmat = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	XMVECTOR camPos = XMVectorSet(m_pos.x, m_pos.y, m_pos.z, 0.0f);
	XMVECTOR camLookAt = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	camLookAt = XMVector3TransformCoord(camLookAt, rotmat);
	camLookAt = camPos + camLookAt;

	XMVECTOR camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	camUp = XMVector3TransformCoord(camUp, rotmat);

	m_viewMatrix = XMMatrixLookAtLH(camPos, camLookAt, camUp);
}

void Camera::MoveForward(bool keydown)
{
	//”величиваем скорость до определённого предела при удержании клавиши, уменьшаем при отпускании
	if (keydown){
		m_forwardSpeed += m_frameTime * 0.001f;

		if (m_forwardSpeed > (m_frameTime * 0.03f))
			m_forwardSpeed = m_frameTime * 0.03f;
	}else{
		m_forwardSpeed -= m_frameTime * 0.0007f;
		if (m_forwardSpeed < 0.0f) m_forwardSpeed = 0.0f;
	}

	float radians = m_rot.y * 0.0174532925f; 
	m_pos.x += sinf(radians) * m_forwardSpeed;
	m_pos.z += cosf(radians) * m_forwardSpeed;
}

void Camera::MoveBackward(bool keydown)
{
	if (keydown){
		m_backwardSpeed += m_frameTime * 0.001f;

		if (m_backwardSpeed > (m_frameTime * 0.03f))
			m_backwardSpeed = m_frameTime * 0.03f;
	}else{
		m_backwardSpeed -= m_frameTime * 0.0007f;
		if (m_backwardSpeed < 0.0f) m_backwardSpeed = 0.0f;
	}

	float radians = m_rot.y * 0.0174532925f;
	m_pos.x -= sinf(radians) * m_backwardSpeed;
	m_pos.z -= cosf(radians) * m_backwardSpeed;
}

void Camera::MoveLeft(bool keydown)
{
	if (keydown){
		m_leftSpeed += m_frameTime * 0.001f;

		if (m_leftSpeed > (m_frameTime * 0.03f))
			m_leftSpeed = m_frameTime * 0.03f;
	}else{
		m_leftSpeed -= m_frameTime* 0.0007f;
		if (m_leftSpeed < 0.0f) m_leftSpeed = 0.0f;
	}

	float rad = m_rot.y * 0.0174532925f;
	m_pos.x -= cosf(rad) * m_leftSpeed;
	m_pos.z -= sinf(rad) * m_leftSpeed;
}

void Camera::MoveRight(bool keydown)
{
	if (keydown){
		m_rightSpeed += m_frameTime * 0.001f;

		if (m_rightSpeed > (m_frameTime * 0.03f))
			m_rightSpeed = m_frameTime * 0.03f;
	}else{
		m_rightSpeed -= m_frameTime* 0.0007f;
		if (m_rightSpeed < 0.0f) m_rightSpeed = 0.0f;
	}

	float rad = m_rot.y * 0.0174532925f;
	m_pos.x += cosf(rad) * m_rightSpeed;
	m_pos.z += sinf(rad) * m_rightSpeed;
}

void Camera::MoveUpward(bool keydown)
{
	if (keydown){
		m_upwardSpeed += m_frameTime * 0.003f;

		if (m_upwardSpeed > (m_frameTime * 0.03f))
			m_upwardSpeed = m_frameTime * 0.03f;
	}else{
		m_upwardSpeed -= m_frameTime * 0.002f;
		if (m_upwardSpeed < 0.0f) m_upwardSpeed = 0.0f;
	}

	m_pos.y += m_upwardSpeed;
}

void Camera::MoveDownward(bool keydown)
{
	if (keydown){
		m_downwardSpeed += m_frameTime * 0.003f;

		if (m_downwardSpeed > (m_frameTime * 0.03f))
			m_downwardSpeed = m_frameTime * 0.03f;
	}else{
		m_downwardSpeed -= m_frameTime * 0.002f;
		if (m_downwardSpeed < 0.0f)	m_downwardSpeed = 0.0f;
	}

	m_pos.y -= m_downwardSpeed;
}

void Camera::RotateLeft(bool keydown)
{
	if (keydown){
		m_leftTurnSpeed += m_frameTime * 0.01f;

		if (m_leftTurnSpeed > (m_frameTime * 0.15f))
			m_leftTurnSpeed = m_frameTime * 0.15f;
	}else{
		m_leftTurnSpeed -= m_frameTime* 0.005f;
		if (m_leftTurnSpeed < 0.0f)	m_leftTurnSpeed = 0.0f;
	}

	m_rot.y -= m_leftTurnSpeed;
	if (m_rot.y < 0.0f) m_rot.y = 360.0f; //+= 360.0f
	/*if (m_rot.y < 0.0f)
		m_rot.y += 360.0f;*/
}


void Camera::RotateRight(bool keydown)
{
	if (keydown){
		m_rightTurnSpeed += m_frameTime * 0.01f;

		if (m_rightTurnSpeed > (m_frameTime * 0.15f))
			m_rightTurnSpeed = m_frameTime * 0.15f;
	}else{
		m_rightTurnSpeed -= m_frameTime* 0.005f;
		if (m_rightTurnSpeed < 0.0f) m_rightTurnSpeed = 0.0f;
	}

	m_rot.y += m_rightTurnSpeed;
	if (m_rot.y > 360.0f) m_rot.y = 0.0f; //-=360.0f
	/*if (m_rot.y > 360.0f)
		m_rot.y -= 360.0f;*/
}

void Camera::LookUpward(bool keydown)
{
	if (keydown){
		m_lookUpSpeed += m_frameTime * 0.01f;

		if (m_lookUpSpeed > (m_frameTime * 0.15f))
			m_lookUpSpeed = m_frameTime * 0.15f;
	}else{
		m_lookUpSpeed -= m_frameTime* 0.005f;
		if (m_lookUpSpeed < 0.0f) m_lookUpSpeed = 0.0f;
	}

	m_rot.x -= m_lookUpSpeed;
	if (m_rot.x > 90.0f) m_rot.x = 90.0f;
}

void Camera::LookDownward(bool keydown)
{
	if (keydown){
		m_lookDownSpeed += m_frameTime * 0.01f;

		if (m_lookDownSpeed > (m_frameTime * 0.15f))
			m_lookDownSpeed = m_frameTime * 0.15f;
	}else{
		m_lookDownSpeed -= m_frameTime* 0.005f;
		if (m_lookDownSpeed < 0.0f) m_lookDownSpeed = 0.0f;
	}

	m_rot.x += m_lookDownSpeed;
	if (m_rot.x < -90.0f) m_rot.x = -90.0f;
}


void Camera::SetPos(float x, float y, float z)
{
	m_pos.x = x; m_pos.y = y; m_pos.z = z;
}

void Camera::SetRot(float x, float y, float z)
{
	m_rot.x = x; m_rot.y = y; m_rot.z = z;
}

CXMMATRIX Camera::GetViewMatrix()
{
	return m_viewMatrix;
}
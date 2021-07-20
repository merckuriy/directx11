#pragma once

#include "Timer.h"

namespace D3D11Framework{

	class Camera
	{
	public:
		Camera();

		void Render();

		void MoveForward(bool keydown);
		void MoveBackward(bool keydown);
		void MoveLeft(bool keydown);
		void MoveRight(bool keydown);
		void MoveUpward(bool keydown);
		void MoveDownward(bool keydown);

		void RotateLeft(bool keydown);
		void RotateRight(bool keydown);

		void LookUpward(bool keydown);
		void LookDownward(bool keydown);

		void SetPos(float x, float y, float z);
		void SetRot(float x, float y, float z);

		XMFLOAT3 GetPos(){ return m_pos; }
		XMFLOAT3 GetRot(){ return m_rot; }

		CXMMATRIX GetViewMatrix();

	private:
		Timer m_time;
		XMMATRIX m_viewMatrix;
		XMFLOAT3 m_pos;
		XMFLOAT3 m_rot;

		float m_frameTime;
		float m_forwardSpeed, m_backwardSpeed;
		float m_upwardSpeed, m_downwardSpeed;
		float m_leftTurnSpeed, m_rightTurnSpeed;
		float m_leftSpeed, m_rightSpeed;
		float m_lookUpSpeed, m_lookDownSpeed;
	};
}
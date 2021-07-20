#pragma once

namespace D3D11Framework
{
	class Frustum
	{
	public:
		//Обнвляет пирамида отсечения.
		void ConstructFrustum(float screenDepth, 
			CXMMATRIX projectionMatrix, CXMMATRIX viewMatrix);

		//Проверка нахождения точки/куба/сферы/параллелепипеда в пирамиде обзора (отсечения).
		bool CheckPoint(float x, float y, float z);
		bool CheckCube(float xCenter, float yCenter, float zCenter, float size);
		bool CheckSphere(float xCenter, float yCenter, float zCenter, float radius);
		bool CheckRectangle(float xCenter, float yCenter, float zCenter,
			float xSize, float ySize, float zSize);

	private:
		XMVECTOR m_planes[6];
	};
}
#pragma once
#include "Render.h"

namespace D3D11Framework
{

	class Image
	{
	public:
		Image(Render *render);

		bool Init(const wchar_t *textureFilename, int bitmapWidth, int bitmapHeight);
		void Draw(int positionX, int positionY);
		void Close();

	private:
		bool m_InitBuffers();
		void m_RenderBuffers();
		void m_SetShaderParameters(float x, float y);
		void m_RenderShader();

		Render *m_render;

		Shader *m_shader;
		ID3D11Buffer *m_vertexBuffer;
		ID3D11Buffer *m_indexBuffer;
		ID3D11Buffer *m_constantBuffer;

		int m_bitmapWidth, m_bitmapHeight;
		int m_previousPosX, m_previousPosY;
	};
}
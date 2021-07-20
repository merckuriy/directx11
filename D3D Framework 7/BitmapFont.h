//�������� �� �������� � ���������� �������� �� ���. � ����� �� ���������.
#pragma once

#include "Render.h"

namespace D3D11Framework
{
	struct VertexFont
	{
		XMFLOAT3 pos;
		XMFLOAT2 tex;
	};

	class BitmapFont
	{
	private:
		struct CharDesc
		{
			CharDesc() : srcX(0), srcY(0), srcW(0), srcH(0), xOff(0), yOff(0), xAdv(0) {}

			//xAdv - ������� ��� ��������� �� x, �� ������ ������� �� ����������.
			short srcX, srcY, srcW, srcH, xOff, yOff, xAdv;
		};

		struct ConstantBuffer
		{
			XMMATRIX WVP;
		};

		struct PixelBufferType
		{
			XMFLOAT4 pixelColor; //������������ ��� ����� ������.
		};

	public:
		BitmapFont(Render *render);

		bool Init(char *fontFilename);
		void Draw(unsigned int index, float r, float g, float b, float x, float y);
		//������ ������ ������ ����������� ������.
		void BuildVertexArray(VertexFont *vertices, int numvert,
			const wchar_t *sentence);
		void Close();

	private:
		//��������� ��������� ���������� � ������.
		bool m_parse(char *fontFilename);
		void m_SetShaderParameters(float r, float g, float b, float x, float y);

		Render *m_render;

		Shader *m_shader;
		ID3D11Buffer *m_constantBuffer;
		ID3D11Buffer *m_pixelBuffer;
		
		unsigned short m_WidthTex;
		unsigned short m_HeightTex;
		std::wstring m_file;
		std::map<int, CharDesc> m_Chars; //�������� �������� (int = id �������).
	};
}

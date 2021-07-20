#pragma once

#include "Render.h"

namespace D3D11Framework{

	class Shader
	{
	public:
		Shader(Render *render);
		//Добавляет свойства вершины (до 8). Вызвать до CreateShader.
		void AddInputElementDesc(const char *SemanticName, DXGI_FORMAT format);
		//Более гибкий метод задания вершины. 
		void AddInputElementDesc(const char *SemanticName, unsigned int SemanticIndex, 
			DXGI_FORMAT format, unsigned int InputSlot = 0, bool AlignedByteOffset = true, 
			D3D11_INPUT_CLASSIFICATION InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA, 
			unsigned int InstanceDataStepRate = 0 );

		bool CreateShader(wchar_t *namevs, wchar_t *nameps);
		bool AddTexture(const wchar_t *name);

		void Draw();
		void Close();

	private:
		HRESULT m_compileshaderfromfile(WCHAR* FileName, LPCSTR EntryPoint, LPCSTR ShaderModel, ID3DBlob** ppBlobOut);

		Render *m_render;

		ID3D11VertexShader *m_vertexShader;
		ID3D11PixelShader *m_pixelShader;
		ID3D11InputLayout *m_layout;
		std::vector<ID3D11ShaderResourceView *> m_textures;
		D3D11_INPUT_ELEMENT_DESC *m_layoutformat;
		unsigned int m_numlayout;
	};

}

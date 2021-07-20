#include "stdafx.h"
#include "BitmapFont.h"
#include <fstream>
#include "macros.h"
#include "Util.h"
#include "Shader.h"
#include "Buffer.h"

using namespace D3D11Framework;
using namespace std;


BitmapFont::BitmapFont(Render *render)
{
	m_render = render;
	m_shader = nullptr;
	m_constantBuffer = nullptr;
	m_pixelBuffer = nullptr;
	m_WidthTex = m_HeightTex = 0;
}

bool BitmapFont::Init(char *fontFilename)
{
	if(!m_parse(fontFilename)) return false;

	m_shader = new Shader(m_render);
	if(!m_shader) return false;

	if(!m_shader->AddTexture(m_file.c_str())) return false;
	m_shader->AddInputElementDesc("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	m_shader->AddInputElementDesc("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	if(!m_shader->CreateShader(L"BitmapFont.vs", L"BitmapFont.ps")) return false;

	m_constantBuffer = Buffer::CreateConstantBuffer(m_render->m_pd3dDevice,
		sizeof(ConstantBuffer), false);
	if(!m_constantBuffer) return false;

	m_pixelBuffer = Buffer::CreateConstantBuffer(m_render->m_pd3dDevice,
		sizeof(PixelBufferType), false);
	if(!m_pixelBuffer) return false;

	return true;
}

bool BitmapFont::m_parse(char *fontFilename)
{
	ifstream fin;
	fin.open(fontFilename);
	if(fin.fail()) return false;

	string Line;
	string Read, Key, Value;
	size_t i;

	while(!fin.eof()){

		std::stringstream LineStream;
		std::getline(fin, Line);
		LineStream << Line; //ввод Line в поток LineStream.

		LineStream >> Read; //читаем по словам.
		if(Read == "common"){
			while(!LineStream.eof()){
				//Используем stringstream как конвертер строки в число.
				std::stringstream Converter;
				LineStream >> Read;
				i = Read.find('=');
				Key = Read.substr(0, i);
				Value = Read.substr(i + 1);

				Converter << Value;
				if(Key == "scaleW") Converter >> m_WidthTex;
				else if(Key == "scaleH") Converter >> m_HeightTex;
			}
		} else if(Read == "page"){
			while(!LineStream.eof()){
				std::stringstream Converter;
				LineStream >> Read;
				i = Read.find('=');
				Key = Read.substr(0, i);
				Value = Read.substr(i + 1);

				std::string str;
				Converter << Value;
				if(Key == "file"){
					Converter >> str;
					wchar_t *name = CharToWChar(const_cast<char*>(str.substr(1, Value.length() - 2).c_str()));
					m_file = name;
					_DELETE_ARRAY(name);
				}
			}
		} else if(Read == "char"){
			unsigned short CharID = 0;
			CharDesc chard;

			while(!LineStream.eof()){
				std::stringstream Converter;
				LineStream >> Read;
				i = Read.find('=');
				Key = Read.substr(0, i);
				Value = Read.substr(i + 1);

				Converter << Value;
				if(Key == "id") Converter >> CharID;
				else if(Key == "x") Converter >> chard.srcX;
				else if(Key == "y") Converter >> chard.srcY;
				else if(Key == "width") Converter >> chard.srcW;
				else if(Key == "height") Converter >> chard.srcH;
				else if(Key == "xoffset") Converter >> chard.xOff;
				else if(Key == "yoffset") Converter >> chard.yOff;
				else if(Key == "xadvance") Converter >> chard.xAdv;
			}
			m_Chars.insert(std::pair<int, CharDesc>(CharID, chard));
		}
	}

	fin.close();

	return true;
}

void BitmapFont::BuildVertexArray(VertexFont *vertices, int numvert, const wchar_t *sentence)
{
	int numLetters = static_cast<int>(wcslen(sentence)); //символов в строке
	//число букв не должно быть больше доступных вершин.
	if(numLetters * 4 > numvert) numLetters = numvert/4;

	//Верхний левый угол.
	float drawX = static_cast<float>(m_render->m_width) / 2 * -1;
	float drawY = static_cast<float>(m_render->m_height) / 2;

	int index = 0;
	for(int i = 0; i < numLetters; i++){
		//неявное преобразование wchar_t в int (его код).
		float СharX = m_Chars[sentence[i]].srcX;
		float СharY = m_Chars[sentence[i]].srcY;
		float Width = m_Chars[sentence[i]].srcW;
		float Height = m_Chars[sentence[i]].srcH;
		float OffsetX = m_Chars[sentence[i]].xOff;
		float OffsetY = m_Chars[sentence[i]].yOff;

		//коорд. на экране
		float left = drawX + OffsetX;
		float right = left + Width;
		float top = drawY - OffsetY;
		float bottom = top - Height;
		//коорд. на текстуре
		float lefttex = СharX / m_WidthTex;
		float righttex = (СharX + Width) / m_WidthTex;
		float toptex = СharY / m_HeightTex;
		float bottomtex = (СharY + Height) / m_HeightTex;

		vertices[index].pos = XMFLOAT3(left, top, 0.0f);
		vertices[index].tex = XMFLOAT2(lefttex, toptex);
		index++;
		vertices[index].pos = XMFLOAT3(right, top, 0.0f);
		vertices[index].tex = XMFLOAT2(righttex, toptex);
		index++;
		vertices[index].pos = XMFLOAT3(left, bottom, 0.0f);
		vertices[index].tex = XMFLOAT2(lefttex, bottomtex);
		index++;	
		vertices[index].pos = XMFLOAT3(right, bottom, 0.0f);
		vertices[index].tex = XMFLOAT2(righttex, bottomtex);
		index++;
		
		drawX += m_Chars[sentence[i]].xAdv;
	}
}

void BitmapFont::Draw(unsigned int index, float r, float g, float b, float x, float y)
{
	m_SetShaderParameters(r, g, b, x, y);

	m_shader->Draw();
	m_render->m_pImmediateContext->DrawIndexed(index, 0, 0);
}

void BitmapFont::m_SetShaderParameters(float r, float g, float b, float x, float y)
{
	XMMATRIX objmatrix = XMMatrixTranslation(x, -y, 0); //смещение текста.
	XMMATRIX wvp = objmatrix*m_render->m_Ortho;
	ConstantBuffer cb;
	cb.WVP = XMMatrixTranspose(wvp);
	m_render->m_pImmediateContext->UpdateSubresource(m_constantBuffer, 0, NULL, &cb, 0, 0);
	m_render->m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_constantBuffer);

	//Задаём цвет текста (через константный буфер пиксельного шейдера)
	PixelBufferType pb;
	pb.pixelColor = XMFLOAT4(r, g, b, 1.0f);
	m_render->m_pImmediateContext->UpdateSubresource(m_pixelBuffer, 0, NULL, &pb, 0, 0);
	m_render->m_pImmediateContext->PSSetConstantBuffers(0, 1, &m_pixelBuffer);
}

void BitmapFont::Close()
{
	_RELEASE(m_constantBuffer);
	_RELEASE(m_pixelBuffer);
	_CLOSE(m_shader);
}
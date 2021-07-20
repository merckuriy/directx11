#include "stdafx.h"
#include "StaticMesh.h"
#include "ms3dspec.h"
#include <fstream>
#include "macros.h"
#include "Util.h"
#include "Shader.h"
#include "Buffer.h"

using namespace D3D11Framework;
using namespace std;

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
};

struct ConstantBuffer
{
	XMMATRIX WVP;
};

StaticMesh::StaticMesh(Render *render)
{
	m_render = render;
	m_shader = nullptr;
	m_vertexBuffer = nullptr;
	m_indexBuffer = nullptr;
	m_constantBuffer = nullptr;
}

bool StaticMesh::Init(wchar_t *name)
{
	Identity();

	m_shader = new Shader(m_render);
	if(!m_shader) return false;

	m_shader->AddInputElementDesc("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	m_shader->AddInputElementDesc("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	if(!m_shader->CreateShader(L"mesh.vs", L"mesh.ps")) return false;

	if(!m_loadMS3DFile(name)) return false;

	return true;
}


bool StaticMesh::m_loadMS3DFile(wchar_t *Filename)
{
	MS3DVertex *pMS3DVertices = nullptr;
	MS3DTriangle *pMS3DTriangles = nullptr;
	MS3DGroup *pMS3DGroups = nullptr;
	MS3DMaterial *pMS3DMaterials = nullptr;

	ifstream fin;
	MS3DHeader header;

	fin.open(Filename, std::ios::binary);
	fin.read(reinterpret_cast<char*>(&(header)), sizeof(header));
	if(header.version != 3 && header.version != 4) return false;

	fin.read(reinterpret_cast<char*>(&(nNumVertices)), sizeof(word));
	pMS3DVertices = new MS3DVertex[nNumVertices];
	fin.read(reinterpret_cast<char*>(pMS3DVertices), nNumVertices * sizeof(MS3DVertex));

	fin.read(reinterpret_cast<char*>(&(nNumTriangles)), sizeof(word));
	pMS3DTriangles = new MS3DTriangle[nNumTriangles];
	fin.read(reinterpret_cast<char*>(pMS3DTriangles), nNumTriangles * sizeof(MS3DTriangle));
	
	fin.read(reinterpret_cast<char*>(&(nNumGroups)), sizeof(word));
	pMS3DGroups = new MS3DGroup[nNumGroups];
	for(int i = 0; i < nNumGroups; i++){
		fin.read(reinterpret_cast<char*>(&(pMS3DGroups[i].flags)), sizeof(byte));
		fin.read(reinterpret_cast<char*>(&(pMS3DGroups[i].name)), sizeof(char[32]));
		fin.read(reinterpret_cast<char*>(&(pMS3DGroups[i].numtriangles)), sizeof(word));
		word numtri = pMS3DGroups[i].numtriangles;
		pMS3DGroups[i].triangleIndices = new word[numtri];
		fin.read(reinterpret_cast<char*>(pMS3DGroups[i].triangleIndices), numtri * sizeof(word));
		fin.read(reinterpret_cast<char*>(&(pMS3DGroups[i].materialIndex)), sizeof(char));
	}

	fin.read(reinterpret_cast<char*>(&(nNumMaterials)), sizeof(word));
	pMS3DMaterials = new MS3DMaterial[nNumMaterials];
	fin.read(reinterpret_cast<char*>(pMS3DMaterials), nNumMaterials * sizeof(MS3DMaterial));
		
	fin.close();

	//Заполняем наши данные на основе считанных.
	m_indexCount = nNumTriangles * 3;
	WORD *indices = new WORD[m_indexCount];
	if(!indices) return false;
	Vertex *vertices = new Vertex[nNumVertices];
	if(!vertices) return false;

	//Индексы вершин по каждому треугольнику.
	for(int i = 0; i < nNumTriangles; i++){
		indices[3*i+0] = pMS3DTriangles[i].vertexIndices[0];
		indices[3*i+1] = pMS3DTriangles[i].vertexIndices[1];
		indices[3*i+2] = pMS3DTriangles[i].vertexIndices[2];
	}

	//Позиции вершин и координаты текстуры.
	for(int i = 0; i < nNumVertices; i++){
		vertices[i].Pos.x = pMS3DVertices[i].vertex[0];
		vertices[i].Pos.y = pMS3DVertices[i].vertex[1];
		vertices[i].Pos.z = pMS3DVertices[i].vertex[2];

		//s,t - коорд. текстуры вершин.
		for(int j = 0; j < nNumTriangles; j++){
			if(i == pMS3DTriangles[j].vertexIndices[0]){
				vertices[i].Tex.x = pMS3DTriangles[j].s[0];
				vertices[i].Tex.y = pMS3DTriangles[j].t[0];
			} else if(i == pMS3DTriangles[j].vertexIndices[1]){
				vertices[i].Tex.x = pMS3DTriangles[j].s[1];
				vertices[i].Tex.y = pMS3DTriangles[j].t[1];
			} else if(i == pMS3DTriangles[j].vertexIndices[2]){
				vertices[i].Tex.x = pMS3DTriangles[j].s[2];
				vertices[i].Tex.y = pMS3DTriangles[j].t[2];
			} else continue;
			break;
		}
	}
	
	//Загружаем указанную текстуру
	wchar_t *name = CharToWChar(pMS3DMaterials[0].texture);
	if(!m_shader->AddTexture(name)) return false;
	_DELETE_ARRAY(name);

	_DELETE_ARRAY(pMS3DMaterials);
	if(pMS3DGroups != nullptr){
		for(int i = 0; i < nNumGroups; i++)
			_DELETE_ARRAY(pMS3DGroups[i].triangleIndices);
		_DELETE_ARRAY(pMS3DGroups);
	}
	_DELETE_ARRAY(pMS3DTriangles);
	_DELETE_ARRAY(pMS3DVertices);

	m_vertexBuffer = Buffer::CreateVertexBuffer(m_render->m_pd3dDevice,
		sizeof(Vertex)*nNumVertices, false, vertices);
	if(!m_vertexBuffer) return false;

	m_indexBuffer = Buffer::CreateIndexBuffer(m_render->m_pd3dDevice,
		sizeof(unsigned short)*m_indexCount, false, indices);
	if(!m_indexBuffer) return false;

	m_constantBuffer = Buffer::CreateConstantBuffer(m_render->m_pd3dDevice,
		sizeof(ConstantBuffer), false);
	if(!m_constantBuffer) return false;

	_DELETE_ARRAY(vertices);
	_DELETE_ARRAY(indices);

	return true;
}

void StaticMesh::Draw(CXMMATRIX viewmatrix)
{
	m_RenderBuffers(); //Привязка буферов и установка топологии.
	m_SetShaderParameters(viewmatrix); //Работа с константным буфером.
	m_RenderShader();
}

void StaticMesh::m_RenderBuffers()
{
	unsigned int stride = sizeof(Vertex);
	unsigned int offset = 0;
	m_render->m_pImmediateContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	m_render->m_pImmediateContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	m_render->m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void StaticMesh::m_SetShaderParameters(CXMMATRIX viewmatrix)
{
	XMMATRIX WVP = m_objMatrix * viewmatrix * m_render->m_Projection;
	ConstantBuffer cb;
	cb.WVP = XMMatrixTranspose(WVP);
	m_render->m_pImmediateContext->UpdateSubresource(m_constantBuffer, 0, NULL, &cb, 0, 0);
	m_render->m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_constantBuffer);
}

void StaticMesh::m_RenderShader()
{
	m_shader->Draw();
	m_render->m_pImmediateContext->DrawIndexed(m_indexCount, 0, 0);
}

void StaticMesh::Close()
{

	_RELEASE(m_indexBuffer);
	_RELEASE(m_vertexBuffer);
	_RELEASE(m_constantBuffer);
	_CLOSE(m_shader);
}

void StaticMesh::Translate(float x, float y, float z)
{
	m_objMatrix *= XMMatrixTranslation(x, y, z);
}

void StaticMesh::Rotate(float angle, float x, float y, float z)
{
	XMVECTOR v = XMVectorSet(x, y, z, 0.0f);
	m_objMatrix *= XMMatrixRotationAxis(v, angle);
}

void StaticMesh::Scale(float x, float y, float z)
{
	m_objMatrix *= XMMatrixScaling(x, y, z);
}

void StaticMesh::Identity()
{
	m_objMatrix = XMMatrixIdentity();
}
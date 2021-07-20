#include "StaticMesh.h"
#include "3dsspec.h"
#include <fstream>

using namespace std;

struct Vertex
{
	XMFLOAT3 Pos; //x,y,z
	XMFLOAT2 Tex; //x,y
};

struct ConstantBuffer
{
	XMMATRIX WVP;
};

wchar_t * ToString(const char* mbString)
{
	int len = 0;
	size_t returnValue;
	len = static_cast<int>(strlen(mbString)) + 1;
	wchar_t *ucString = new wchar_t[len];
	mbstowcs_s(&returnValue, ucString, len, mbString, len);
	return ucString;
}

StaticMesh::StaticMesh()
{
	m_vertexBuffer = nullptr;
	m_indexBuffer = nullptr;
	m_vertexShader = nullptr;
	m_pixelShader = nullptr;
	m_layout = nullptr;
	m_pConstantBuffer = nullptr;
	m_samplerState = nullptr;
	m_texture = nullptr;
	m_rot = 0.0f;
}

bool StaticMesh::Init(MyRender *render, wchar_t *name)
{
	m_objMatrix = XMMatrixIdentity();
	m_render = render;
	if(!m_loadMS3DFile(name)) return false;
	if(!m_InitShader(L"mesh.vs", L"mesh.ps")) return false;

	return true;
}

//���� ��� ����� � ��� �� ������ (+ ��������� ������� �������).
bool checkChunks(ifstream &fin){
	word chunk_id = 0;
	dword chunk_len = 0;

	while(!fin.eof()){
		fin.read(reinterpret_cast<char*>(&chunk_id), 2);
		if(fin.eof()) break;
		fin.read(reinterpret_cast<char*>(&chunk_len), 4);
		Log::Get()->Debug("Id: %4X, �����: %3u, �����: %x", chunk_id, chunk_len, static_cast<dword>(fin.tellg()) - 6);
		fin.ignore(chunk_len - 6);
	}
	return true;
}

//���� ���� � ��� �� ������, �.�. 1 ��� = ��� �����. ��������� ����� ��������� ��� �����.
bool toChunk(ifstream &fin, word id){
	word chunk_id = 0;
	dword chunk_len = 0;

	while(!fin.eof()){
		fin.read(reinterpret_cast<char*>(&chunk_id), 2);
		if(fin.eof()){ Log::Get()->Err("%X �� ������.", id); return false; }
		fin.read(reinterpret_cast<char*>(&chunk_len), 4);
		
		if(chunk_id != id){
			fin.ignore(chunk_len - 6); //��� � �����, ��� �� ������ � ����� �����.
		} else return true;
	} 
	Log::Get()->Err("%X �� ������.", id); return false;
}

//������ ���� � ��������� ���������, ��������� � ������������.
bool StaticMesh::m_loadMS3DFile(wchar_t *Filename)
{
	//Log::Get()->Debug("%u", static_cast<int>(fin.tellg()));
	//Log::Get()->Debug("Id: %X", chunk_id);
	//fin.seekg(4, ios_base::cur); //ios_base::cur - ������� ������� ��������� ���������. seekg - �������� �� 4 �����.
	//checkChunks(fin);

	word chunk_id;	//������������� ����� (2 �����)
	dword chunk_len;//�������� � ���������� ����� (�� chunk_id) (4 �����)
	
	nNumVertices = 0;
	nNumTriangles = 0;
	VertexPos *pVertices = nullptr;
	Textcoord *pTextcoords = nullptr;
	Triangle *pTriangles = nullptr;

	ifstream fin;
	fin.open(Filename, std::ios::binary | ios_base::beg); //beg - ������� ������� � ������.

	fin.read(reinterpret_cast<char*>(&chunk_id), 2);
	//fin.read(reinterpret_cast<char*>(&chunk_len), 4);
	fin.ignore(4); 

	if(chunk_id != MAIN3DS){ 
		Log::Get()->Err("������ �������. ��������� ���� .3ds"); 
		return false; 
	}


	fin.ignore(6); //#0x0002
	fin.read(reinterpret_cast<char*>(&chunk_len), 4);
	Log::Get()->Debug("������ 3DS: %u", chunk_len);

	fin.ignore(6); //#0x3D3D CHUNK_3D_EDITOR(2+4)
	
	fin.ignore(6); //#0x3D3E
	fin.read(reinterpret_cast<char*>(&chunk_len), 4);
	Log::Get()->Debug("������ ����: %u", chunk_len);
	//checkChunks(fin);

	fin.ignore(6); //MATERIAL_BLOCK #0xAFFF

	/*	A000, �����: 19
		A010, �����: 24
		A020, �����: 24
		A030, �����: 24
		A040, �����: 14
		A041, �����: 14
		A050, �����: 14
		A052, �����: 14
		A053, �����: 14
		A100, �����:  8	//Render type. "3" ������.
		A084, �����: 14
		A08A, �����:  6
		A087, �����: 10
		A200, �����: 71 //Texture map.
	*/

	fin.ignore(6); //#0xA000 : Material name
	string matName;
	getline(fin, matName, '\0');
	Log::Get()->Print("��� ���������: %s", matName.c_str());

	toChunk(fin, 0xA100);
	fin.read(reinterpret_cast<char*>(&chunk_id), 2);
	Log::Get()->Debug("��� �������: %u", chunk_id);

	

	toChunk(fin, MAT_TEXMAP);

	/*	0030, �����:  8 // Percent chunk (�� 0 �� 100)
		A300, �����: 19 // (MAT_MAPNAME) ��� ����� ��������. 
		A351, �����:  8	// Mapping parameters
		A353, �����: 10 // ������� �������� (������ float)
		A354, �����: 10 // V scale
		A356, �����: 10 // U scale	
	*/
	
	toChunk(fin, MAT_MAPNAME); //#0xA300
	string matFileName;
	getline(fin, matFileName, '\0');
	Log::Get()->Print("��� ����� ���������: %s", matFileName.c_str());
		
	/*toChunk(fin, 0x0100); float one_unit; //1.0
	fin.read(reinterpret_cast<char*>(&one_unit), 4);*/
	
	toChunk(fin, OBJECT_BLOCK); //#0x4000

	string objName;
	getline(fin, objName, '\0');
	Log::Get()->Print("��� �������: %s", objName.c_str());

	toChunk(fin, OBJ_TRIMESH);
	
	//������� ������.
	fin.read(reinterpret_cast<char*>(&chunk_id), 2);
	fin.ignore(4);
	if(chunk_id != CHUNK_VERTLIST){ Log::Get()->Err("������� �� �������."); return false; }
	fin.read(reinterpret_cast<char*>(&nNumVertices), 2);
	pVertices = new VertexPos[nNumVertices];
	fin.read(reinterpret_cast<char*>(pVertices), nNumVertices * sizeof(VertexPos));

	//���������� �������.
	if(!toChunk(fin, CHUNK_MAPCOORDS)){ Log::Get()->Err("���������� ������� �� �������.");  return false; }
	fin.ignore(2);
	pTextcoords = new Textcoord[nNumVertices];
	fin.read(reinterpret_cast<char*>(pTextcoords), nNumVertices * sizeof(Textcoord));
	
	//������� ������ �� ���������.
	if(!toChunk(fin, CHUNK_FACEDESC)){ Log::Get()->Err("�������� �� �������.");  return false; }
	fin.read(reinterpret_cast<char*>(&nNumTriangles), 2);
	pTriangles = new Triangle[nNumTriangles];
	fin.read(reinterpret_cast<char*>(pTriangles), nNumTriangles * sizeof(Triangle));
	
	//������� ��������� �� ������� ���������. 
	/*if(!toChunk(fin, CHUNK_FACEMAT)){ Log::Get()->Err("��������� �� �������.");  return false; }
	getline(fin, matName, '\0');
	fin.read(reinterpret_cast<char*>(&nNumEntries), 2);
	Log::Get()->Print("�������� ���������: %s, ����� ���������: %u", matName.c_str(), nNumEntries);*/

	//��������� ���� ������.
	m_indexCount = nNumTriangles * 3;
	WORD *indices = new WORD[m_indexCount];
	if(!indices) return false;
	Vertex *vertices = new Vertex[nNumVertices];
	if(!vertices) return false;

	//������� ������.
	for(int i = 0; i < nNumTriangles; i++){
		indices[3 * i + 0] = pTriangles[i].v0;	
		indices[3 * i + 1] = pTriangles[i].v1;
		indices[3 * i + 2] = pTriangles[i].v2;
	}

	//�������� � ���������� ������. (���������� � ���� ���������)
	for(int i = 0; i < nNumVertices; i++){
		vertices[i].Pos.x = -pVertices[i].x; //�������� - �.�. �������� ����� ������ ����.
		vertices[i].Pos.y = -pVertices[i].y; //�������� -
		vertices[i].Pos.z = -pVertices[i].z; //������ ����.

		vertices[i].Tex.x = pTextcoords[i].u;
		vertices[i].Tex.y = pTextcoords[i].v;
	}

	//��������� ��������� ��������
	if(!m_LoadTextures(ToString(matFileName.c_str()))) return false;

	_DELETE_ARRAY(pTriangles);
	_DELETE_ARRAY(pVertices);
	_DELETE_ARRAY(pTextcoords);

	
	//����� ������.
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) * nNumVertices;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;

	//m_pd3dDevice �� Render ������������ MyRender, � ������� ������ StaticMesh.
	HRESULT hr = m_render->m_pd3dDevice->CreateBuffer(&bd, &InitData, &m_vertexBuffer);
	if(FAILED(hr)) return false;

	//��������� �����.
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD)* m_indexCount;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = indices;
	hr = m_render->m_pd3dDevice->CreateBuffer(&bd, &InitData, &m_indexBuffer);
	if(FAILED(hr)) return false;

	//����������� �����.
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = m_render->m_pd3dDevice->CreateBuffer(&bd, NULL, &m_pConstantBuffer);
	if(FAILED(hr)) return false;


	_DELETE_ARRAY(vertices);
	_DELETE_ARRAY(indices);

	return true;
}

bool StaticMesh::m_LoadTextures(wchar_t *textureFilename)
{
	HRESULT hr = D3DX11CreateShaderResourceViewFromFile(m_render->m_pd3dDevice, 
		textureFilename, NULL, NULL, &m_texture, NULL);
	if(FAILED(hr)) return false;

	return true;
}

//������������� ���������� � ����������� ��������.
bool StaticMesh::m_InitShader(wchar_t* vsFilename, wchar_t* psFilename)
{
	//��������� ������
	ID3DBlob* VSBuffer = NULL;
	HRESULT hr = m_render->m_compileshaderfromfile(vsFilename, "VS", "vs_4_0", &VSBuffer);
	if(FAILED(hr)){ Log::Get()->Err("�� ������� �������������� \"VS\" shader.fx"); return false; }

	hr = m_render->m_pd3dDevice->CreateVertexShader(VSBuffer->GetBufferPointer(), VSBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if(FAILED(hr)){ _RELEASE(VSBuffer); Log::Get()->Err("�� ������� ������� ��������� ������"); return false; }

	//���������� ������.
	ID3DBlob* PSBuffer = NULL;
	hr = m_render->m_compileshaderfromfile(psFilename, "PS", "ps_4_0", &PSBuffer);
	if(FAILED(hr)){ Log::Get()->Err("�� ������� �������������� \"PS\" shader.fx"); return false; }

	hr = m_render->m_pd3dDevice->CreatePixelShader(PSBuffer->GetBufferPointer(), PSBuffer->GetBufferSize(), NULL, &m_pixelShader);
	_RELEASE(PSBuffer);
	if(FAILED(hr)){ Log::Get()->Err("�� ������� ������� ���������� ������"); return false; }

	//�������� � �������� ������� �����. (������� ��-�������, ��� ������)
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0; //����� ���������� ��� ��������� �� ����� �� �������
	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	//������� ������� ��������������� ����� �����������. �������� ������������, ���� ����������.
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	UINT numElements = ARRAYSIZE(polygonLayout);// sizeof(polygonLayout) / sizeof(polygonLayout[0]);
	
	hr = m_render->m_pd3dDevice->CreateInputLayout(polygonLayout,
		numElements, VSBuffer->GetBufferPointer(), VSBuffer->GetBufferSize(), &m_layout);
	if(FAILED(hr)) return false;
	_RELEASE(VSBuffer);

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP; 
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.MipLODBias = 0.0f; //�������� mip ������.
	sampDesc.MaxAnisotropy = 1; //���� ������� ����������� (1-16)
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	//sampDesc.BorderColor[4] //���� ������� (4 �������), ���� ������ TEXTURE_ADDRESS_BORDER
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = m_render->m_pd3dDevice->CreateSamplerState(&sampDesc, &m_samplerState);
	if(FAILED(hr)) return false;

	return true;
}

//��������� (���������� �� Draw)
void StaticMesh::Render()
{
	m_rot += .00003f; //.00008
	//if(m_rot > 6.26f) m_rot = 0.0f;

	m_RenderBuffers(); //�������� ������� � ��������� ���������.
	m_SetShaderParameters(); //��������� �������� � ��������.
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

void StaticMesh::m_SetShaderParameters()
{
	XMVECTOR rotaxis = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX Scale = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	XMMATRIX Rotation = XMMatrixRotationAxis(rotaxis, m_rot);
	//XMMATRIX RotationZ = XMMatrixRotationZ(m_rot / 1.2);

	m_objMatrix = Scale*Rotation; //*RotationZ

	XMMATRIX WVP = m_objMatrix * m_render->m_View * m_render->m_Projection;
	ConstantBuffer cb;
	cb.WVP = XMMatrixTranspose(WVP);
	m_render->m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, NULL, &cb, 0, 0);
	m_render->m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	m_render->m_pImmediateContext->PSSetShaderResources(0, 1, &m_texture);
}

void StaticMesh::m_RenderShader()
{
	m_render->m_pImmediateContext->IASetInputLayout(m_layout);
	m_render->m_pImmediateContext->VSSetShader(m_vertexShader, NULL, 0);
	m_render->m_pImmediateContext->PSSetShader(m_pixelShader, NULL, 0);
	m_render->m_pImmediateContext->PSSetSamplers(0, 1, &m_samplerState);
	m_render->m_pImmediateContext->DrawIndexed(m_indexCount, 0, 0);
}

void StaticMesh::Close()
{
	_RELEASE(m_texture);
	_RELEASE(m_indexBuffer);
	_RELEASE(m_vertexBuffer);
	_RELEASE(m_pConstantBuffer);
	_RELEASE(m_samplerState);
	_RELEASE(m_layout);
	_RELEASE(m_pixelShader);
	_RELEASE(m_vertexShader);
}
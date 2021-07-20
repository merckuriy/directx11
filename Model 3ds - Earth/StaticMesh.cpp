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

//Ищет все блоки в том же уровне (+ последние верхних уровней).
bool checkChunks(ifstream &fin){
	word chunk_id = 0;
	dword chunk_len = 0;

	while(!fin.eof()){
		fin.read(reinterpret_cast<char*>(&chunk_id), 2);
		if(fin.eof()) break;
		fin.read(reinterpret_cast<char*>(&chunk_len), 4);
		Log::Get()->Debug("Id: %4X, Длина: %3u, Адрес: %x", chunk_id, chunk_len, static_cast<dword>(fin.tellg()) - 6);
		fin.ignore(chunk_len - 6);
	}
	return true;
}

//Ищет блок в том же уровне, т.к. 1 шаг = его длине. Остановка после прочтения его длины.
bool toChunk(ifstream &fin, word id){
	word chunk_id = 0;
	dword chunk_len = 0;

	while(!fin.eof()){
		fin.read(reinterpret_cast<char*>(&chunk_id), 2);
		if(fin.eof()){ Log::Get()->Err("%X не найден.", id); return false; }
		fin.read(reinterpret_cast<char*>(&chunk_len), 4);
		
		if(chunk_id != id){
			fin.ignore(chunk_len - 6); //Имя и длина, так же входят в длину чанка.
		} else return true;
	} 
	Log::Get()->Err("%X не найден.", id); return false;
}

//Читаем файл и заполняем структуры, описанные в спецификации.
bool StaticMesh::m_loadMS3DFile(wchar_t *Filename)
{
	//Log::Get()->Debug("%u", static_cast<int>(fin.tellg()));
	//Log::Get()->Debug("Id: %X", chunk_id);
	//fin.seekg(4, ios_base::cur); //ios_base::cur - текущая позиция файлового указателя. seekg - Смещение на 4 байта.
	//checkChunks(fin);

	word chunk_id;	//Идентификатор блока (2 байта)
	dword chunk_len;//Смещение к следующему блоку (от chunk_id) (4 байта)
	
	nNumVertices = 0;
	nNumTriangles = 0;
	VertexPos *pVertices = nullptr;
	Textcoord *pTextcoords = nullptr;
	Triangle *pTriangles = nullptr;

	ifstream fin;
	fin.open(Filename, std::ios::binary | ios_base::beg); //beg - позиция курсора в начало.

	fin.read(reinterpret_cast<char*>(&chunk_id), 2);
	//fin.read(reinterpret_cast<char*>(&chunk_len), 4);
	fin.ignore(4); 

	if(chunk_id != MAIN3DS){ 
		Log::Get()->Err("Ошибка формата. Требуется файл .3ds"); 
		return false; 
	}


	fin.ignore(6); //#0x0002
	fin.read(reinterpret_cast<char*>(&chunk_len), 4);
	Log::Get()->Debug("Версия 3DS: %u", chunk_len);

	fin.ignore(6); //#0x3D3D CHUNK_3D_EDITOR(2+4)
	
	fin.ignore(6); //#0x3D3E
	fin.read(reinterpret_cast<char*>(&chunk_len), 4);
	Log::Get()->Debug("Версия меша: %u", chunk_len);
	//checkChunks(fin);

	fin.ignore(6); //MATERIAL_BLOCK #0xAFFF

	/*	A000, Длина: 19
		A010, Длина: 24
		A020, Длина: 24
		A030, Длина: 24
		A040, Длина: 14
		A041, Длина: 14
		A050, Длина: 14
		A052, Длина: 14
		A053, Длина: 14
		A100, Длина:  8	//Render type. "3" обычно.
		A084, Длина: 14
		A08A, Длина:  6
		A087, Длина: 10
		A200, Длина: 71 //Texture map.
	*/

	fin.ignore(6); //#0xA000 : Material name
	string matName;
	getline(fin, matName, '\0');
	Log::Get()->Print("Имя материала: %s", matName.c_str());

	toChunk(fin, 0xA100);
	fin.read(reinterpret_cast<char*>(&chunk_id), 2);
	Log::Get()->Debug("Тип рендера: %u", chunk_id);

	

	toChunk(fin, MAT_TEXMAP);

	/*	0030, Длина:  8 // Percent chunk (от 0 до 100)
		A300, Длина: 19 // (MAT_MAPNAME) Имя файла текстуры. 
		A351, Длина:  8	// Mapping parameters
		A353, Длина: 10 // Процент размытия (видимо float)
		A354, Длина: 10 // V scale
		A356, Длина: 10 // U scale	
	*/
	
	toChunk(fin, MAT_MAPNAME); //#0xA300
	string matFileName;
	getline(fin, matFileName, '\0');
	Log::Get()->Print("Имя файла материала: %s", matFileName.c_str());
		
	/*toChunk(fin, 0x0100); float one_unit; //1.0
	fin.read(reinterpret_cast<char*>(&one_unit), 4);*/
	
	toChunk(fin, OBJECT_BLOCK); //#0x4000

	string objName;
	getline(fin, objName, '\0');
	Log::Get()->Print("Имя объекта: %s", objName.c_str());

	toChunk(fin, OBJ_TRIMESH);
	
	//Позиции вершин.
	fin.read(reinterpret_cast<char*>(&chunk_id), 2);
	fin.ignore(4);
	if(chunk_id != CHUNK_VERTLIST){ Log::Get()->Err("Вершины не найдены."); return false; }
	fin.read(reinterpret_cast<char*>(&nNumVertices), 2);
	pVertices = new VertexPos[nNumVertices];
	fin.read(reinterpret_cast<char*>(pVertices), nNumVertices * sizeof(VertexPos));

	//Координаты текстур.
	if(!toChunk(fin, CHUNK_MAPCOORDS)){ Log::Get()->Err("Координаты текстур не найдены.");  return false; }
	fin.ignore(2);
	pTextcoords = new Textcoord[nNumVertices];
	fin.read(reinterpret_cast<char*>(pTextcoords), nNumVertices * sizeof(Textcoord));
	
	//Индексы вершин на полигонах.
	if(!toChunk(fin, CHUNK_FACEDESC)){ Log::Get()->Err("Полигоны не найдены.");  return false; }
	fin.read(reinterpret_cast<char*>(&nNumTriangles), 2);
	pTriangles = new Triangle[nNumTriangles];
	fin.read(reinterpret_cast<char*>(pTriangles), nNumTriangles * sizeof(Triangle));
	
	//Индексы полигонов из данного материала. 
	/*if(!toChunk(fin, CHUNK_FACEMAT)){ Log::Get()->Err("Материалы не найдены.");  return false; }
	getline(fin, matName, '\0');
	fin.read(reinterpret_cast<char*>(&nNumEntries), 2);
	Log::Get()->Print("Название материала: %s, Число полигонов: %u", matName.c_str(), nNumEntries);*/

	//Заполняем свои данные.
	m_indexCount = nNumTriangles * 3;
	WORD *indices = new WORD[m_indexCount];
	if(!indices) return false;
	Vertex *vertices = new Vertex[nNumVertices];
	if(!vertices) return false;

	//Индексы вершин.
	for(int i = 0; i < nNumTriangles; i++){
		indices[3 * i + 0] = pTriangles[i].v0;	
		indices[3 * i + 1] = pTriangles[i].v1;
		indices[3 * i + 2] = pTriangles[i].v2;
	}

	//Текстура и координаты вершин. (объединяем в одну структуру)
	for(int i = 0; i < nNumVertices; i++){
		vertices[i].Pos.x = -pVertices[i].x; //временно - т.к. текстура кверх ногами была.
		vertices[i].Pos.y = -pVertices[i].y; //временно -
		vertices[i].Pos.z = -pVertices[i].z; //Меняем знак.

		vertices[i].Tex.x = pTextcoords[i].u;
		vertices[i].Tex.y = pTextcoords[i].v;
	}

	//Загружаем указанную текстуру
	if(!m_LoadTextures(ToString(matFileName.c_str()))) return false;

	_DELETE_ARRAY(pTriangles);
	_DELETE_ARRAY(pVertices);
	_DELETE_ARRAY(pTextcoords);

	
	//Буфер вершин.
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) * nNumVertices;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;

	//m_pd3dDevice от Render унаследовано MyRender, с которым дружит StaticMesh.
	HRESULT hr = m_render->m_pd3dDevice->CreateBuffer(&bd, &InitData, &m_vertexBuffer);
	if(FAILED(hr)) return false;

	//Индексный буфер.
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD)* m_indexCount;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = indices;
	hr = m_render->m_pd3dDevice->CreateBuffer(&bd, &InitData, &m_indexBuffer);
	if(FAILED(hr)) return false;

	//Константный буфер.
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

//Инициализация вершинного и пиксельного шейдеров.
bool StaticMesh::m_InitShader(wchar_t* vsFilename, wchar_t* psFilename)
{
	//Вершинный шейдер
	ID3DBlob* VSBuffer = NULL;
	HRESULT hr = m_render->m_compileshaderfromfile(vsFilename, "VS", "vs_4_0", &VSBuffer);
	if(FAILED(hr)){ Log::Get()->Err("Не удалось скомпилировать \"VS\" shader.fx"); return false; }

	hr = m_render->m_pd3dDevice->CreateVertexShader(VSBuffer->GetBufferPointer(), VSBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if(FAILED(hr)){ _RELEASE(VSBuffer); Log::Get()->Err("Не удалось создать вершинный шейдер"); return false; }

	//Пиксельный шейдер.
	ID3DBlob* PSBuffer = NULL;
	hr = m_render->m_compileshaderfromfile(psFilename, "PS", "ps_4_0", &PSBuffer);
	if(FAILED(hr)){ Log::Get()->Err("Не удалось скомпилировать \"PS\" shader.fx"); return false; }

	hr = m_render->m_pd3dDevice->CreatePixelShader(PSBuffer->GetBufferPointer(), PSBuffer->GetBufferSize(), NULL, &m_pixelShader);
	_RELEASE(PSBuffer);
	if(FAILED(hr)){ Log::Get()->Err("Не удалось создать пиксельный шейдер"); return false; }

	//Описание и создание формата ввода. (немного по-другому, чем раньше)
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0; //Число экземляров для отрисовки по таким же данными
	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	//Текущий элемент непосредственно после предыдущего. Включает выравнивание, если необходимо.
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
	sampDesc.MipLODBias = 0.0f; //Смещение mip уровня.
	sampDesc.MaxAnisotropy = 1; //макс степерь анизотропии (1-16)
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	//sampDesc.BorderColor[4] //цвет границы (4 стороны), если указан TEXTURE_ADDRESS_BORDER
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = m_render->m_pd3dDevice->CreateSamplerState(&sampDesc, &m_samplerState);
	if(FAILED(hr)) return false;

	return true;
}

//Отрисовка (вызывается из Draw)
void StaticMesh::Render()
{
	m_rot += .00003f; //.00008
	//if(m_rot > 6.26f) m_rot = 0.0f;

	m_RenderBuffers(); //Привязка буферов и установка топологии.
	m_SetShaderParameters(); //Выполняем действия с объектом.
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
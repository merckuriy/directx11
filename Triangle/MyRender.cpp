#include "MyRender.h"
#include <xnamath.h>
#include <D3Dcompiler.h>

struct SimpleVertex
{
	XMFLOAT3 Pos; //Вершина. Содержит 3 числа float - координаты.
};


MyRender::MyRender()
{
	m_pVertexShader = nullptr;
	m_pPixelShader = nullptr;
	m_pVertexLayout = nullptr;
	m_pVertexBuffer = nullptr;
}

HRESULT MyRender::m_compileshaderfromfile(WCHAR* FileName, LPCSTR EntryPoint,
	LPCSTR ShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;
	//см. ниже
	/*hr = D3DX11CompileFromFile(FileName, NULL, NULL, EntryPoint, 
		ShaderModel, 0, 0, NULL, ppBlobOut, NULL, NULL);*/
	hr = D3DCompileFromFile(FileName, NULL, NULL, EntryPoint,
		ShaderModel, 0, 0, ppBlobOut, NULL);

	return hr;
}

bool MyRender::Init(HWND hwnd)
{
	//Вершинный шейдер.
	HRESULT hr = S_OK;
	ID3DBlob* pVSBlob = NULL;
	hr = m_compileshaderfromfile(L"shader.fx", "VS", "vs_4_0", &pVSBlob);
	if(FAILED(hr)){
		Log::Get()->Err("Не удалось скомпилировать \"VS\" shader.fx. Примеч: файл должен быть в папке с программой.");
		return false;
	}

	//3(ID3D11ClassLinkage) - Объект(класс), хранящий созданные шейдеры.
	hr = m_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), NULL, &m_pVertexShader);
	if(FAILED(hr)){
		Log::Get()->Err("Не удалось создать вершинный шейдер");
		_RELEASE(pVSBlob);
		return false;
	}

	//см ниже. Описание входного формата вершин для видеокарты для 1-ой стадии конвеера. layout - макет.
	D3D11_INPUT_ELEMENT_DESC layout[] = 
	{	//R32G32B32_FLOAT - 3 32-битных float.
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(layout);

	//Создание объекта формата.
	hr = m_pd3dDevice->CreateInputLayout(layout, numElements,
		pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexLayout);

	_RELEASE(pVSBlob);
	if(FAILED(hr)) return false;

	//Подключение формата к стадии input-assembler.
	m_pImmediateContext->IASetInputLayout(m_pVertexLayout);


	//Пиксельный шейдер.
	ID3DBlob* pPSBlob = NULL;
	hr = m_compileshaderfromfile(L"shader.fx", "PS", "ps_4_0", &pPSBlob);
	if(FAILED(hr)){
		Log::Get()->Err("Не удалось скомпилировать \"PS\" shader.fx. Примеч: файл должен быть в папке с программой.");
		return false;
	}

	hr = m_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),
		pPSBlob->GetBufferSize(), NULL, &m_pPixelShader);
	if(FAILED(hr)){
		Log::Get()->Err("Не удалось создать пиксельный шейдер");
		_RELEASE(pPSBlob);
		return false;
	}

	//Создание буфера вершин. (для 1-ой стадии)
	SimpleVertex vertices[] =
	{
		XMFLOAT3(0.0f, 0.5f, 0.5f),
		XMFLOAT3(0.5f, -0.5f, 0.5f),
		XMFLOAT3(-0.5f, -0.5f, 0.5f)
	};

	D3D11_BUFFER_DESC bd; //Описатель буфера.
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT; //Использование буфера. Чтение/запись, CPU/GPU. (read and write GPU)
	bd.ByteWidth = sizeof(SimpleVertex)*3; //Размер буфера в байтах
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER; //Тип буфера (вершинный)
	bd.CPUAccessFlags = 0; //права чтение/записи CPU. (если установлен соответствующий Usage)
	//bd.MiscFlags //Различные(miscellaneous) флаги для разных типов буферов.
	//bd.StructureByteStride //Размер каждого элемента буфера (если используется как структурированный буфер)

	//Устанавливает данные для инициализации
	D3D11_SUBRESOURCE_DATA Data;
	ZeroMemory(&Data, sizeof(Data));
	Data.pSysMem = vertices; //Указатель на данные.
	//Data.SysMemPitch - расстояние (в байтах) от начала одной линии текстуры до следующей. (только для 2D и 3D текстур).
	//Data.SysMemSlicePitch - расстояние от начала одного уровня глубины до другого. (только для 3D текстур).

	//CreateBuffer - Создаёт вершинный или индексный или shader-constant буфер.
	hr = m_pd3dDevice->CreateBuffer(&bd, &Data, &m_pVertexBuffer);
	if(FAILED(hr)) return false;

	//Установка вершинного буфера в input-assembler. см ниже.
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	m_pImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//Установка топологии (тип, правила рендеринга вершин) примитива
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return true;
}

bool MyRender::Draw()
{
	//2 -(ID3D11ClassInstance) - массив экземпляров классов HLSL. 3 -их кол-во.
	m_pImmediateContext->VSSetShader(m_pVertexShader, NULL, 0); //2 стадия
	m_pImmediateContext->PSSetShader(m_pPixelShader, NULL, 0); //9 стадия
	m_pImmediateContext->Draw(3, 0);

	return true;
}

void MyRender::Close()
{
	_RELEASE(m_pVertexBuffer);
	_RELEASE(m_pVertexLayout);
	_RELEASE(m_pVertexShader);
	_RELEASE(m_pPixelShader);
}

/*//Компилирует шейдер или эффект из файла. D3DX11 - устарели для Windows 8. 
Рекомендуется использовать D3DCompileFromFile из WinAPI (требует D3DCompiler.lib).
HRESULT D3DX11CompileFromFile( //vs D3DCompileFromFile (-)-нет параметров.
  _In_   LPCTSTR pSrcFile, //Имя файла (широкоформатное)
  _In_   const D3D10_SHADER_MACRO *pDefines, //Макроопределения шейдера (#define)
  _In_   LPD3D10INCLUDE pInclude, //Интерфейс, позволяющий переписывать методы открытия/закрытия файла шейдера.
  _In_   LPCSTR pEntrypoint, //функция входа в файле шейдера. При компиляции эффекта не используется.
  _In_   LPCSTR pProfile, //Модель шейдера (тип и версия:2,3,4,5) или эффекта.
  _In_   UINT Flags1, //Опции компиляции шейдеров
  _In_   UINT Flags2, //Опции компиляции эффектов
  _In_   ID3DX11ThreadPump *pPump(-), //Интерфейс для ассинхронной загрузки компиляции.
  _Out_  ID3D10Blob **ppShader, //Скомпилированный шейдер
  _Out_  ID3D10Blob **ppErrorMsgs, //Список ошибок и предупреждений, возникших во время компиляции
  _Out_  HRESULT *pHResult(-)
);

//Входной формат вершин для 1-ой стадии конвеера (input-assembler).
struct D3D11_INPUT_ELEMENT_DESC {
LPCSTR                     SemanticName; //Назначение этой характеристики (позиция, цвет и т.д.)
UINT                       SemanticIndex;//Индекс. Добавляется к имени. Т.к. могут быть одинаковые назначения.
DXGI_FORMAT                Format;
UINT                       InputSlot; //Идентификатор слота input-assembler. от 0 до 15.
UINT                       AlignedByteOffset; //Смещение в байтах этой характеристики.
D3D11_INPUT_CLASSIFICATION InputSlotClass; //тип данных (вершина, либо экземпляр) в слоте ввода (input slot).
UINT                       InstanceDataStepRate; //Число одинаковых экземпляров. (если указаны данные экземпляра).
} D3D11_INPUT_ELEMENT_DESC;

//Привязывает вершинный буфер к ассемблеру ввода (input-assembler).
void IASetVertexBuffers(
[in]  UINT StartSlot, //начальный слот для привязки буфера вершин. (max от 16 до 32 слотов).
[in]  UINT NumBuffers, //число вершинных буферов в массиве.
[in]  ID3D11Buffer *const *ppVertexBuffers, //массив вершинных буферов
[in]  const UINT *pStrides, //массив ширины буферов.
[in]  const UINT *pOffsets //массив смещений буферов вершин. 
смещение - это расстояние от начала буфера до первого используемого в нём элемента.
*/
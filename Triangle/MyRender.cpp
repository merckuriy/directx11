#include "MyRender.h"
#include <xnamath.h>
#include <D3Dcompiler.h>

struct SimpleVertex
{
	XMFLOAT3 Pos; //�������. �������� 3 ����� float - ����������.
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
	//��. ����
	/*hr = D3DX11CompileFromFile(FileName, NULL, NULL, EntryPoint, 
		ShaderModel, 0, 0, NULL, ppBlobOut, NULL, NULL);*/
	hr = D3DCompileFromFile(FileName, NULL, NULL, EntryPoint,
		ShaderModel, 0, 0, ppBlobOut, NULL);

	return hr;
}

bool MyRender::Init(HWND hwnd)
{
	//��������� ������.
	HRESULT hr = S_OK;
	ID3DBlob* pVSBlob = NULL;
	hr = m_compileshaderfromfile(L"shader.fx", "VS", "vs_4_0", &pVSBlob);
	if(FAILED(hr)){
		Log::Get()->Err("�� ������� �������������� \"VS\" shader.fx. ������: ���� ������ ���� � ����� � ����������.");
		return false;
	}

	//3(ID3D11ClassLinkage) - ������(�����), �������� ��������� �������.
	hr = m_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), NULL, &m_pVertexShader);
	if(FAILED(hr)){
		Log::Get()->Err("�� ������� ������� ��������� ������");
		_RELEASE(pVSBlob);
		return false;
	}

	//�� ����. �������� �������� ������� ������ ��� ���������� ��� 1-�� ������ ��������. layout - �����.
	D3D11_INPUT_ELEMENT_DESC layout[] = 
	{	//R32G32B32_FLOAT - 3 32-������ float.
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(layout);

	//�������� ������� �������.
	hr = m_pd3dDevice->CreateInputLayout(layout, numElements,
		pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexLayout);

	_RELEASE(pVSBlob);
	if(FAILED(hr)) return false;

	//����������� ������� � ������ input-assembler.
	m_pImmediateContext->IASetInputLayout(m_pVertexLayout);


	//���������� ������.
	ID3DBlob* pPSBlob = NULL;
	hr = m_compileshaderfromfile(L"shader.fx", "PS", "ps_4_0", &pPSBlob);
	if(FAILED(hr)){
		Log::Get()->Err("�� ������� �������������� \"PS\" shader.fx. ������: ���� ������ ���� � ����� � ����������.");
		return false;
	}

	hr = m_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),
		pPSBlob->GetBufferSize(), NULL, &m_pPixelShader);
	if(FAILED(hr)){
		Log::Get()->Err("�� ������� ������� ���������� ������");
		_RELEASE(pPSBlob);
		return false;
	}

	//�������� ������ ������. (��� 1-�� ������)
	SimpleVertex vertices[] =
	{
		XMFLOAT3(0.0f, 0.5f, 0.5f),
		XMFLOAT3(0.5f, -0.5f, 0.5f),
		XMFLOAT3(-0.5f, -0.5f, 0.5f)
	};

	D3D11_BUFFER_DESC bd; //��������� ������.
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT; //������������� ������. ������/������, CPU/GPU. (read and write GPU)
	bd.ByteWidth = sizeof(SimpleVertex)*3; //������ ������ � ������
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER; //��� ������ (���������)
	bd.CPUAccessFlags = 0; //����� ������/������ CPU. (���� ���������� ��������������� Usage)
	//bd.MiscFlags //���������(miscellaneous) ����� ��� ������ ����� �������.
	//bd.StructureByteStride //������ ������� �������� ������ (���� ������������ ��� ����������������� �����)

	//������������� ������ ��� �������������
	D3D11_SUBRESOURCE_DATA Data;
	ZeroMemory(&Data, sizeof(Data));
	Data.pSysMem = vertices; //��������� �� ������.
	//Data.SysMemPitch - ���������� (� ������) �� ������ ����� ����� �������� �� ���������. (������ ��� 2D � 3D �������).
	//Data.SysMemSlicePitch - ���������� �� ������ ������ ������ ������� �� �������. (������ ��� 3D �������).

	//CreateBuffer - ������ ��������� ��� ��������� ��� shader-constant �����.
	hr = m_pd3dDevice->CreateBuffer(&bd, &Data, &m_pVertexBuffer);
	if(FAILED(hr)) return false;

	//��������� ���������� ������ � input-assembler. �� ����.
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	m_pImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//��������� ��������� (���, ������� ���������� ������) ���������
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return true;
}

bool MyRender::Draw()
{
	//2 -(ID3D11ClassInstance) - ������ ����������� ������� HLSL. 3 -�� ���-��.
	m_pImmediateContext->VSSetShader(m_pVertexShader, NULL, 0); //2 ������
	m_pImmediateContext->PSSetShader(m_pPixelShader, NULL, 0); //9 ������
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

/*//����������� ������ ��� ������ �� �����. D3DX11 - �������� ��� Windows 8. 
������������� ������������ D3DCompileFromFile �� WinAPI (������� D3DCompiler.lib).
HRESULT D3DX11CompileFromFile( //vs D3DCompileFromFile (-)-��� ����������.
  _In_   LPCTSTR pSrcFile, //��� ����� (���������������)
  _In_   const D3D10_SHADER_MACRO *pDefines, //���������������� ������� (#define)
  _In_   LPD3D10INCLUDE pInclude, //���������, ����������� ������������ ������ ��������/�������� ����� �������.
  _In_   LPCSTR pEntrypoint, //������� ����� � ����� �������. ��� ���������� ������� �� ������������.
  _In_   LPCSTR pProfile, //������ ������� (��� � ������:2,3,4,5) ��� �������.
  _In_   UINT Flags1, //����� ���������� ��������
  _In_   UINT Flags2, //����� ���������� ��������
  _In_   ID3DX11ThreadPump *pPump(-), //��������� ��� ������������ �������� ����������.
  _Out_  ID3D10Blob **ppShader, //���������������� ������
  _Out_  ID3D10Blob **ppErrorMsgs, //������ ������ � ��������������, ��������� �� ����� ����������
  _Out_  HRESULT *pHResult(-)
);

//������� ������ ������ ��� 1-�� ������ �������� (input-assembler).
struct D3D11_INPUT_ELEMENT_DESC {
LPCSTR                     SemanticName; //���������� ���� �������������� (�������, ���� � �.�.)
UINT                       SemanticIndex;//������. ����������� � �����. �.�. ����� ���� ���������� ����������.
DXGI_FORMAT                Format;
UINT                       InputSlot; //������������� ����� input-assembler. �� 0 �� 15.
UINT                       AlignedByteOffset; //�������� � ������ ���� ��������������.
D3D11_INPUT_CLASSIFICATION InputSlotClass; //��� ������ (�������, ���� ���������) � ����� ����� (input slot).
UINT                       InstanceDataStepRate; //����� ���������� �����������. (���� ������� ������ ����������).
} D3D11_INPUT_ELEMENT_DESC;

//����������� ��������� ����� � ���������� ����� (input-assembler).
void IASetVertexBuffers(
[in]  UINT StartSlot, //��������� ���� ��� �������� ������ ������. (max �� 16 �� 32 ������).
[in]  UINT NumBuffers, //����� ��������� ������� � �������.
[in]  ID3D11Buffer *const *ppVertexBuffers, //������ ��������� �������
[in]  const UINT *pStrides, //������ ������ �������.
[in]  const UINT *pOffsets //������ �������� ������� ������. 
�������� - ��� ���������� �� ������ ������ �� ������� ������������� � �� ��������.
*/
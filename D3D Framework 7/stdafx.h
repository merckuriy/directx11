#pragma once

#include <clocale> //Локализация
#include <ctime> //Дата и время

#include <sstream>
#include <string>
#include <list>
#include <vector>
#include <map>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <d3d11.h>
#include <d3dx11.h>
#include <D3Dcompiler.h>
#include <xnamath.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")
#ifdef _DEBUG
#	pragma comment(lib, "d3dx11d.lib")
#else
#	pragma comment(lib, "d3dx11.lib")
#endif
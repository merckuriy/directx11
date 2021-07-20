#include "Fps.h"
#include "D3D11_Framework.h"
//#include <mmsystem.h> //Для системного времени. Пока используется время программы.

//#pragma comment(lib, "winmm.lib")

void FpsClass::Init()
{
	m_fps = 0;
	m_count = 0;
	m_startTime = clock(); //timeGetTime()
	return;
}

void FpsClass::Frame()
{
	m_count++;
	if(/*timeGetTime()*/clock() >= (m_startTime + 1000)){
		m_fps = m_count;
		m_count = 0;

		m_startTime = clock(); //timeGetTime()
	}
}

int FpsClass::GetFps()
{
	return m_fps;
}
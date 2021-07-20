#include "stdafx.h"
#include "Fps.h"

using namespace D3D11Framework;

void FpsClass::Init()
{
	m_fps = 0;
	m_count = 0;
	m_startTime = clock(); //timeGetTime()
}

void FpsClass::Frame()
{
	m_count++;
	//timeGetTime() - ф-я Windows в уроке
	if(clock() >= (m_startTime + 1000)){
		m_fps = m_count;
		m_count = 0;

		m_startTime = clock(); //timeGetTime()
	}
}

int FpsClass::GetFps()
{
	return m_fps;
}
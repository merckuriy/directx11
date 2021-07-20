#include "stdafx.h"
#include "Timer.h"

using namespace D3D11Framework;

bool Timer::Init()
{
	//Частота - импульсов в секунду. фиксированное значение.
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&m_frequency));
	if(m_frequency == 0) return false;

	m_ticksPerMs = static_cast<float>(m_frequency / 1000);
	//Число импульсов.
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&m_startTime));

	return true;
}

void Timer::Frame()
{
	INT64 currentTime;
	float timeDifference;
	
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currentTime));

	timeDifference = static_cast<float>(currentTime - m_startTime);
	m_frameTime = timeDifference / m_ticksPerMs;

	m_startTime = currentTime;
}

float Timer::GetTime()
{
	return m_frameTime;
}
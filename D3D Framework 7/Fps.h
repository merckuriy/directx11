#pragma once

namespace D3D11Framework
{
	class FpsClass
	{
	public:
		void Init();
		void Frame();
		int GetFps();

	private:
		int m_fps, m_count;
		unsigned long m_startTime;
	};
}
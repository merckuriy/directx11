#pragma once

namespace D3D11Framework{

	class Timer
	{
	public:
		bool Init();
		void Frame();
		float GetTime();

	private:
		INT64 m_frequency;
		INT64 m_startTime;
		float m_ticksPerMs;
		float m_frameTime;
	};
}

/*
Int 32 -- (-2,147,483,648 to 2,147,483,647)
Int 64 -- (-9,223,372,036,854,775,808 to 9,223,372,036,854,775,807)
*/
#pragma once

namespace D3D11Framework
{
	class InputMgr;

	struct DescWindow
	{
		DescWindow():caption(L""), width(640), height(480), posx(200), posy(20), resizing(true) {}
	
		int posx, posy;
		std::wstring caption;
		int width, height;
		bool resizing;
	};

	class Window
	{
	public:
		Window();
		
		static Window* Get(){ return m_wndthis; }

		bool Create(const DescWindow &desc);
		void RunEvent();
		void Close();

		void SetInputMgr(InputMgr *inputmgr);

		HWND GetHWND() const{ return m_hwnd; }
		int Window::GetLeft() const{ return m_desc.posx; }
		int Window::GetTop() const{ return m_desc.posy; }
		int Window::GetWidth() const{ return m_desc.width; }
		int Window::GetHeight() const{ return m_desc.height; }
		const std::wstring& GetCaption() const { return m_desc.caption; }

		bool IsExit() const { return m_isexit; } 
		bool IsActive() const { return m_active; }
		bool IsResize()
		{
			bool ret = m_isresize;
			m_isresize = false;
			return ret;
		}

		LRESULT WndProc(HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

	private:
		void m_UpdateWindowState();

		static Window *m_wndthis;

		DescWindow m_desc; 
		InputMgr *m_inputmgr;
		HWND m_hwnd; 
		bool m_isexit; 
		bool m_active; 
		bool m_minimized , m_maximized;
		bool m_isresize; 
	};

	static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
}
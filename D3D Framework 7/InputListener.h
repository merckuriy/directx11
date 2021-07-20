//Слушатель

#pragma once

namespace D3D11Framework
{
	struct MouseEvent{
		MouseEvent(int nx, int ny) : x(nx), y(ny) {}
		int x;
		int y;
	};

	struct MouseEventClick :public MouseEvent{
		MouseEventClick(eMouseKeyCodes b, int nx, int ny) :MouseEvent(nx, ny), btn(b) {}
		
		const eMouseKeyCodes btn;
	};

	struct MouseEventWheel :public MouseEvent{
		MouseEventWheel(int nwheel, int nx, int ny) :MouseEvent(nx, ny), wheel(nwheel) {}

		int wheel;
	};

	struct KeyEvent{
		KeyEvent(wchar_t c, eKeyCodes kc) :wc(c), code(kc) {}

		const wchar_t wc;
		const eKeyCodes code;
	};

	class InputListener
	{
	public:
		virtual bool MousePressed(const MouseEventClick &arg) { return false; }
		virtual bool MouseReleased(const MouseEventClick &arg) { return false; }
		virtual bool MouseWheel(const MouseEventWheel &arg) { return false; }
		virtual bool MouseMove(const MouseEvent &arg) { return false; }

		virtual bool KeyPressed(const KeyEvent &arg) { return false; }
		virtual bool KeyReleased(const KeyEvent &arg) { return false; }
	};

	
}
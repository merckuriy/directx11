#include "MyInput.h"

MyInput::MyInput(MyRender *render)
{
	m_render = render;
}

bool MyInput::KeyPressed(const KeyEvent &arg)
{
	if (arg.code == KEY_UP)
		m_render->m_key_up = true;
	else if (arg.code == KEY_DOWN)
		m_render->m_key_down = true;
	else if (arg.code == KEY_LEFT)
		m_render->m_key_left = true;
	else if (arg.code == KEY_RIGHT)
		m_render->m_key_right = true;
	else if (arg.code == KEY_A)
		m_render->m_key_a = true;
	else if (arg.code == KEY_Z)
		m_render->m_key_z = true;
	else if (arg.code == KEY_S)
		m_render->m_key_s = true;
	else if (arg.code == KEY_X)
		m_render->m_key_x = true;

	return true;
}

bool MyInput::KeyReleased(const KeyEvent &arg)
{
	if (arg.code == KEY_UP)
		m_render->m_key_up = false;
	else if (arg.code == KEY_DOWN)
		m_render->m_key_down = false;
	else if (arg.code == KEY_LEFT)
		m_render->m_key_left = false;
	else if (arg.code == KEY_RIGHT)
		m_render->m_key_right = false;
	else if (arg.code == KEY_A)
		m_render->m_key_a = false;
	else if (arg.code == KEY_Z)
		m_render->m_key_z = false;
	else if (arg.code == KEY_S)
		m_render->m_key_s = false;
	else if (arg.code == KEY_X)
		m_render->m_key_x = false;

	return true;
}

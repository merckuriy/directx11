#include "MyInput.h"

MyInput::MyInput(MyRender *render)
{
	m_render = render;
}

bool MyInput::KeyPressed(const KeyEvent &arg)
{
	if (arg.code == KEY_LEFT) m_render->m_leftcam = true;
	else if (arg.code == KEY_RIGHT) m_render->m_rightcam = true;

	return true;
}

bool MyInput::KeyReleased(const KeyEvent &arg)
{
	if (arg.code == KEY_LEFT) m_render->m_leftcam = false;
	else if (arg.code == KEY_RIGHT) m_render->m_rightcam = false;

	return true;
}
#include "MyRender.h"
#define _CRT_SECURE_NO_WARNINGS

int main()
{
	//Скрываем окно косноли
	HWND hWnd = GetConsoleWindow();
	ShowWindow(hWnd, SW_HIDE);

	Framework framework;
	MyRender *render = new MyRender();

	FrameworkDesc desc;
	desc.render = render;

	framework.Init(desc);

	framework.Run();

	framework.Close();

	return 0;
}
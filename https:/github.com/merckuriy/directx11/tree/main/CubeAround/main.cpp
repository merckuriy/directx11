#include "MyRender.h"

int main()
{
	Framework framework;
	MyRender *render = new MyRender();

	FrameworkDesc desc;
	desc.wnd.width = 640;
	desc.wnd.height = 480;
	desc.render = render;

	framework.Init(desc);

	framework.Run();

	framework.Close();

	return 0;
}
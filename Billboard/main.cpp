#include "MyRender.h"
#include "MyInput.h"

int main()
{
	Framework framework;

	MyRender *render = new MyRender();
	MyInput *input = new MyInput(render);

	FrameworkDesc desc;
	desc.render = render;

	framework.Init(desc);
	framework.AddInputListener(input);
	framework.Run();
	framework.Close();

	delete input;

	return 0;
}
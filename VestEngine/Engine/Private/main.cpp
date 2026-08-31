
#include "Engine.h"

int main()
{
	Engine engine;
	if (engine.initialize())
	{
		engine.launch();
	}
	return 0;
}
#include "Engine.h"
#include "Platform/Platform.h"

int main()
{
	platform::initialize();

	Engine engine;
	if (engine.initialize())
	{
		engine.launch();
	}

	platform::shutdown();
	return 0;
}
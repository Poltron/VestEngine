#pragma once

class Scene;

namespace demoScene
{
	void loadAxis(Scene& inScene, float inLength);
	void loadPrimitivesDemo(Scene& inScene);
	void loadCubesDemo(Scene& inScene);
	void loadAnimals(Scene& inScene, unsigned int inTotal, unsigned inRowSize);
};
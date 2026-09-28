#pragma once

#include "glm/glm.hpp"

class Scene;
struct ResourceHandle;

namespace demoScene
{
	void loadAxis(Scene& inScene, float inLength);

	void loadPrimitivesDemo(Scene& inScene);
	void loadCubesDemo(Scene& inScene);

	void loadAnimals(Scene& inScene, unsigned int inTotal, unsigned inRowSize);
	void createAnimal(Scene& inScene);
	void createAnimal(Scene& inScene, ResourceHandle inModel, const glm::vec3& inPosition);
};
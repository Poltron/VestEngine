#include "ECS/Systems/RespawnSystem.h"

#include "tracy/Tracy.hpp"

#include "Core/Scene.h"
#include "DemoScene.h"

void RespawnSystem::update(Scene& inScene)
{
	ZoneScoped;

	int missingAnimals = 2000 - (int)inScene.getEntityManager().getNumberEntityAlive();
	for (int i = 0; i < missingAnimals; ++i)
	{
		demoScene::createAnimal(inScene);
	}
}
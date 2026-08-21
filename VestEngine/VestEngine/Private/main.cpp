
//
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

//
#include "glad/glad.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

//
#include "Camera.h"
#include "Engine.h"
#include "Managers/ComponentManager.h"
#include "Managers/EntityManager.h"
#include "Managers/ResourcesManager.h"
#include "Resources/Mesh.h"
#include "Resources/Texture.h"
#include "Systems/Renderer.h"
#include "Systems/TransformSystem.h"

int main()
{
	Engine engine;
	if (engine.initialize())
	{
		engine.launch();
	}
	return 0;
}
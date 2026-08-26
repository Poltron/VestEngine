#pragma once

#include <vector>
#include <string>

#include "glm/vec3.hpp"

#include "Managers/ResourcesManager.h"
#include "Resources/Mesh.h"
#include "Systems/ShaderParameterCollection.h"

struct ShaderParameter
{
	std::string name;
};

struct MeshRendererComponent
{
	unsigned int entityID;
	
	ResourceHandle model;
	ResourceHandle shader;

	ShaderParameterCollection shaderParameters;
};
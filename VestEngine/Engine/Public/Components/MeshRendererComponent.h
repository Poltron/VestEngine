#pragma once

#include "Resources/ResourceHandle.h"
#include "Systems/ShaderParameterCollection.h"

#include "Components/Entity.h"

struct MeshRendererComponent 
{
	Entity entity = 0;
	
	ResourceHandle model;
	ResourceHandle shader;

	ShaderParameterCollection shaderParameters;
};
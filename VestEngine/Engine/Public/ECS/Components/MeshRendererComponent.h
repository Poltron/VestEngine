#pragma once

#include "Core/ResourceHandle.h"
#include "Render/ShaderParameterCollection.h"

#include "ECS/Entity.h"

struct MeshRendererComponent 
{
	Entity entity = 0;
	
	ResourceHandle model;
	ResourceHandle shader;

	ShaderParameterCollection shaderParameters;
};
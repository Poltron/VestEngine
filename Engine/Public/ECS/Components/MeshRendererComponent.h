#pragma once

#include "Core/ResourceHandle.h"
#include "Render/ShaderParameterCollection.h"

#include "ECS/Entity.h"

struct MeshRendererComponent 
{
	Entity entity;
	
	ResourceHandle model;
	ResourceHandle shader;
	bool bOutline;

	ShaderParameterCollection shaderParameters;

	MeshRendererComponent()
		: entity(ENTITY_INVALID), model(RESOURCE_INVALID), shader(RESOURCE_INVALID), bOutline(false)
	{
		shaderParameters.clear();
	}
};
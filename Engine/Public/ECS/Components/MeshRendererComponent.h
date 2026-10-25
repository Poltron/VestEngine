#pragma once

#include "Core/ResourceHandle.h"
#include "Render/GPUHandle.h"
#include "Render/ShaderParameterCollection.h"

#include "ECS/Entity.h"

struct MeshRendererComponent 
{
	Entity entity;
	
	ResourceHandle model;
	GPUHandle shader;
	bool bOutline;

	ShaderParameterCollection shaderParameters;

	MeshRendererComponent()
		: entity(ENTITY_INVALID), model(RESOURCE_INVALID), shader({}), bOutline(false)
	{
		shaderParameters.clear();
	}
};